#include "common.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <vector>
#include <stdexcept>
#include <iostream>

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

class Server
{
private:
    const int client_n = 10;
    std::vector<int> clients;
    int sock = 0;
    bool is_running = false;

    int _handle_client(int i);
    void _accept_new();
    void _close_client(int i);
public:
    Server();
    ~Server();

    void run();
};

Server::Server()
{
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (sock < 0) 
        throw std::runtime_error("Error: socket failed");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERV_PORT);
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
        struct timeval interval = {10, 0};
        int max_sock = sock;
        fd_set sock_set;
        FD_ZERO(&sock_set);

        FD_SET(sock, &sock_set);
        for (int sock : clients)
        {
            FD_SET(sock, &sock_set);
            max_sock = MAX(max_sock, sock);
        }

        int code = select(max_sock+1, &sock_set, NULL, NULL, &interval);
        if (code == 0) break;
        else if (code <0)
            throw std::runtime_error("Error: select failed");

        if (FD_ISSET(sock, &sock_set))
            _accept_new();
        
        for (int i = 0; i < clients.size(); i++)
        {
            if (!FD_ISSET(clients[i], &sock_set))
                continue;
            if (_handle_client(i))
                printf("Client disconnected\n");
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
        return 0;
    }    
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
}

int main(void)
{
    Server s = Server();
    printf("Socket linked, server listening\n");
    try
    {
        s.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    /*
    struct sockaddr_in serv_addr;
    std::vector<int> client_sock;
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (sock < 0) 
    {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERV_PORT);
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        close(sock);
        perror("bind failed\n");
        return EXIT_FAILURE;
    }

    if (listen(sock, CLIENT_N))
    {
        close(sock);
        perror("listen failed\n");
        return EXIT_FAILURE;
    }

    printf("Socket linked, server listening\n");
    fd_set sock_set;
    int max_sock;

    signal(SIGINT, sigint_action);
    while (1)
    {
        struct timeval interval = {10, 0};
        max_sock = sock;
        FD_ZERO(&sock_set);

        FD_SET(sock, &sock_set);
        for (int i=0; i<CLIENT_N; i++)
        {
            if (client_sock[i])
            {
                FD_SET(client_sock[i], &sock_set);
                max_sock = MAX(max_sock, client_sock[i]);
            }
        }

        int code = select(max_sock+1, &sock_set, NULL, NULL, &interval);
        if (code == 0)
        {
            close_sockets(client_sock, sock);
            printf("server closed\n");
            return 0;
        }
        else if (code < 0)
        {
            close_sockets(client_sock, sock);
            perror("select failed\n");
            return EXIT_FAILURE;
        }

        if (FD_ISSET(sock, &sock_set))
        {
            int new_sock = accept(sock, NULL, NULL);
            if (new_sock < 0)
            {
                close_sockets(client_sock, sock);
                perror("aceept failed\n");
                return EXIT_FAILURE;
            }
            add_socket(client_sock, new_sock);
        }

        for (int i=0; i<CLIENT_N; i++)
        {
            if (handle_client(client_sock[i], FD_ISSET(client_sock[i], &sock_set))) 
                    printf("Client №%d disconnected\n", i);
            
            // if (client_sock[i] && FD_ISSET(client_sock[i], &sock_set))
            // {
            //     char buf[BUF_SIZE];
            //     int bytes = recvfrom(client_sock[i], buf, sizeof(buf), 0, NULL, NULL);
            //     if (bytes <= 0)
            //     {
            //         printf("Client №%d disconnected\n", i);
            //         close(client_sock[i]);
            //         client_sock[i] = 0;
            //     } 
            //     else 
            //     {
            //         buf[bytes] = '\0';
            //         printf("Server received: %s\n", buf);
            //     }
            // }
        }
    }

    close_sockets(client_sock, sock);
    return 0;
    */
}
