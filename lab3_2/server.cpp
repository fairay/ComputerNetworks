#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <memory>

#include "common.h"
#include "http.hpp"

std::string look_dir(void);

class Server
{
private:
    const uint port;
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
    Server(const Server& s) = default;
    ~Server();

    void run();
    void stop();
};

Server::Server(uint port_): port(port_)
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
    send(clients[i], st.c_str(), st.length(), 0);

    char buf[BUF_SIZE];
    while (!feof(fd)) 
    {
        if ((bytes_read = fread(buf, 1, BUF_SIZE, fd)) > 0)
            send(clients[i], buf, bytes_read, 0);
        else if (bytes_read < 0)
            throw std::runtime_error("Error: file sending  failed");
        else
            break;
    }

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
