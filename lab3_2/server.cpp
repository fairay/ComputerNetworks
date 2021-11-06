#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <vector>
#include <stdexcept>
#include <iostream>

#include "common.h"
#include "http.hpp"

std::string look_dir(void);

class Server
{
private:
    const int client_n = 10;
    struct timeval delay = {60, 0};
    std::vector<int> clients;
    int sock = 0;
    bool is_running = false;

    int _handle_client(int i);
    void _accept_new();
    void _close_client(int i);
public:
    Server(uint port);
    ~Server();

    void run();
    void stop();
};

Server::Server(uint port)
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
}
void Server::stop()
{
    is_running = false;
    for (int sock : clients)
        close(sock);
    clients.clear();

    close(sock);
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
            
            switch (_handle_client(i))
            {
            case 1:
                printf("Client disconnected\n");
                break;
            case 2:
                printf("File not found\n");
                break;
            default:
                break;
            }
        }
    }
}

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

int Server::_handle_client(int i)
{
    std::string s;
    try
    {
        s = _recv_str(clients[i]);
    }
    catch(const std::exception& e)
    {
        _close_client(i);
        return 1;
    }
    
    HttpRequest req(s);
    HttpResponse res;
    std::cout << req.method << std::endl;
    std::cout << req.path << std::endl;
    std::cout << req.version << std::endl;

    std::string path = "static/" + req.path;
    int code = res.from_file(path);

    std::cout << "req.version" << std::endl;
    char buf[BUF_SIZE];
    FILE *fd;
    if (code == 404)
        fd = fopen("static/404.html", "rb"); 
    else
        fd = fopen(path.c_str(), "rb");
    

    std::cout << "req.version" << std::endl;
    int bytes_read;

    std::string st = res.to_string();
    const char* buf2 = st.c_str();
    std::cout << st.length() << std::endl;
    std::cout << buf2 << std::endl;
    send(clients[i], buf2, st.length(), 0);

    std::cout << code << std::endl;

    while (!feof(fd)) 
    {
        if ((bytes_read = fread(&buf, 1, BUF_SIZE, fd)) > 0)
            send(clients[i], buf, bytes_read, 0);
        else if (bytes_read < 0)
            throw std::runtime_error("Error: file sending  failed");
        else
            break;
    }

    std::cout << "req.version last" << std::endl;
    fclose(fd);

    _close_client(i);
    return 1;
}

void Server::_close_client(int i)
{
    close(clients[i]);
    clients[i] = 0;
    clients.erase(clients.begin() + i);
}

void Server::_accept_new() 
{
    int new_sock = accept(sock, NULL, NULL);
    if (new_sock < 0)
        throw std::runtime_error("Error: accept failed");
    clients.push_back(new_sock);
    printf("Client connected\n");
}

int main(void)
{
    try
    {
        Server s = Server(SERV_PORT);
        printf("Socket linked, server listening\n");
        s.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}
