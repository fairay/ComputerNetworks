#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <memory>

#include "common.h"
#include "http.hpp"
#include "threadpoll.hpp"

std::string _recv_str(int sock) 
{
    std::string s("");
    const int bsize = BUF_SIZE;
    char buf[bsize+1];
    int bytes;
    do
    {
        bytes = recv(sock, (void*)buf, bsize, 0);
        buf[bytes] = '\0';

        if (bytes <= 0)
            throw std::runtime_error("Error: recv failed");
        else 
            s += buf;
    } while (bytes == bsize);
    return s;
}

struct handle_result 
{
    int code;
    std::string req_path;
};

struct handle_result handle_client(int sock)
{
    struct handle_result ret;
    std::string req_str;
    try
    {
        req_str = _recv_str(sock);
    }
    catch(const std::exception& e)
    {
        ret.code = 1;
        return ret;
    }
    
    HttpRequest req(req_str);
    HttpResponse res;

    std::string path;
    if (req.path != "")
        path = "static/" + req.path;
    else
        path = "static/index.html";
    int code = res.from_file(path);

    FILE *fd;
    if (code == 404)
        fd = fopen("static/404.html", "rb"); 
    else
        fd = fopen(path.c_str(), "rb");
    

    int bytes_read;

    std::string st = res.to_string();
    send(sock, st.c_str(), st.length(), 0);

    char buf[BUF_SIZE];
    while (!feof(fd)) 
    {
        if ((bytes_read = fread(buf, 1, BUF_SIZE, fd)) > 0)
            send(sock, buf, bytes_read, 0);
        else if (bytes_read < 0)
            throw std::runtime_error("Error: file sending  failed");
        else
            break;
    }

    fclose(fd);
    ret.code = 1;
    ret.req_path = req.path;
    return ret;
}


class Server
{
private:
    const uint port;
    const int client_n = 10;
    const std::string log_path = "stat.log";
    ThreadPool pool;

    struct timeval delay = {60, 0};
    std::vector<int> clients;
    std::vector<sockaddr_in> addr;
    int sock = 0;
    bool is_running = false;

    int _handle_client(int i);
    void _accept_new();
    void _close_client(int i);
    void _log(const struct sockaddr_in &client, std::string req);
public:
    Server(uint port);
    Server(const Server& s) = default;
    ~Server();

    void run();
    void stop();
};

Server::Server(uint port_): port(port_), pool(16)
{
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (sock < 0) 
        throw std::runtime_error("Error: socket failed");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        throw std::runtime_error("Error: bind failed");

    if (listen(sock, client_n))
        throw std::runtime_error("Error: listen failed");
}

Server::~Server()
{
    this->stop();
    std::cout << "Closed\n";
}

void Server::stop()
{
    is_running = false;
    for (int csock : clients)
        if (csock)
            close(sock);
    
    clients.clear();
    addr.clear();

    if (sock)
    {
        close(sock);
        sock = 0;
    }
}

void Server::run() 
{
    is_running = true;
    while (is_running)
    {
        int max_sock = sock;
        fd_set sock_set;
        FD_ZERO(&sock_set);

        FD_SET(sock, &sock_set);
        for (int sock : clients)
        {
            FD_SET(sock, &sock_set);
            max_sock = MAX(max_sock, sock);
        }

        int code = select(max_sock+1, &sock_set, NULL, NULL, &delay);
        if (code == 0) break;
        else if (code <0)
            throw std::runtime_error("Error: select failed");

        if (FD_ISSET(sock, &sock_set))
            _accept_new();
        
        for (int i = 0; i < clients.size(); i++)
        {
            if (!FD_ISSET(clients[i], &sock_set))
                continue;
            
            auto f = pool.schedule(handle_client, clients[i]);
            auto res = f.get();
            _log(addr[i], res.req_path);

            switch (res.code)
            {
            case 1:
                printf("Client disconnected\n");
                _close_client(i--);
                break;
            default:
                break;
            }
        }
    }
}

void Server::_close_client(int i)
{
    close(clients[i]);
    clients[i] = 0;
    clients.erase(clients.begin() + i);
    addr.erase(addr.begin() + i);
}

void Server::_accept_new() 
{
    struct sockaddr_in new_addr;
    socklen_t len = sizeof(sockaddr_in);

    int new_sock = accept(sock, (struct sockaddr *)&new_addr, &len);
    if (new_sock < 0)
        throw std::runtime_error("Error: accept failed");

    clients.push_back(new_sock);
    addr.push_back(new_addr);

    printf("Client connected\n");
}

void Server::_log(const struct sockaddr_in &client, std::string req)
{
    std::ofstream fout(log_path, std::ios::app);

    std::string remote_addr(inet_ntoa(client.sin_addr));
    std::string ext = req.substr(req.find_last_of(".") + 1);
    fout << "Client: " + remote_addr + " -> \t" + ext + "\n";
}



std::shared_ptr<Server> s;

void sigint_action(int signum)
{
    printf("\nInterrupt signal received, closing server\n");
    if (s) 
    {
        s->stop();
    }
    exit(0);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sigint_action);

    uint port = SERV_PORT;
    if (argc == 2)
    {
        sscanf(argv[1], "%ud", &port);
    }
    std::cout << "http://localhost:"<< port << std::endl;

    try
    {
        s = std::shared_ptr<Server>(new Server(port));
        printf("Socket linked, server listening\n");
        s->run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}
