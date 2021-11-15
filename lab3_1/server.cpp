#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <vector>
#include <stdexcept>
#include <iostream>

#include "common.h"

/*
void close_sockets(std::vector<int> &clients, int main_sock)
{
    close(main_sock);

    for (int sock : clients)
        close(sock);
    clients.clear();
}
void add_socket(std::vector<int> &clients, int new_sock) 
{
    clients.push_back(new_sock);
}

void sigint_action(int signum)
{
    printf("\nInterrupt signal received, closing server\n");
    exit(0);
}

int* empty_socket(int *client_sock)
{
    for (int i=0; i<CLIENT_N; i++)
    {
        if (!client_sock[i])
            return &client_sock[i];
    }
    return NULL;
}

int accept_new(std::vector<int> &clients, int main_sock) 
{
    int new_sock = accept(main_sock, NULL, NULL);
    if (new_sock < 0)
        return EXIT_FAILURE;
    add_socket(clients, new_sock);
    return EXIT_SUCCESS;
}
*/

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

int Server::_handle_client(int i)
{
    char buf[BUF_SIZE];
    int sock = this->clients[i];
    int bytes = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
    if (bytes <= 0)
    {
        _close_client(i);
        return 1;
    }  else 
    {
        buf[bytes] = '\0';
        printf("Server received: %s\n", buf);
    }

    file_msg code;
    FILE *fd = fopen(buf, "rb");
    if (fd)
        code = FILE_OK;
    else
        code = NOT_FOUND;

    if (send(sock, &code, sizeof(code), 0) <= 0)
        throw std::runtime_error("Error: send code failed");
    
    if (code != FILE_OK)
        return 2;

    size_t rret, wret;
    int bytes_read;
    while (!feof(fd)) 
    {
        if ((bytes_read = fread(&buf, 1, BUF_SIZE, fd)) > 0)
            send(sock, buf, bytes_read, 0);
        else if (bytes_read < 0)
            throw std::runtime_error("Error: file sending  failed");
        else
            break;
    }
    fclose(fd);
    return 0;
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

    std::string dir = look_dir();
    if (send(new_sock, dir.c_str(), dir.length(), 0) < 0)
        throw std::runtime_error("Error: directory content sending failed");
    printf("Files sended\n");
}

int main(void)
{
    std::string dir = look_dir();
    std::cout << "Avaliable files:" << std::endl << dir;

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
