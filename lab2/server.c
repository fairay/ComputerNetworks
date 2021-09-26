#include "header.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#define CLIENT_N    5

void close_sockets(int *sock_arr, int main_sock)
{
    for (int i=0; i<CLIENT_N; i++)
        close(sock_arr[i]);
    close(main_sock);
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


int main(void)
{
    struct sockaddr_in serv_addr;
    
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

    int client_sock[CLIENT_N];
    for (int i=0; i<CLIENT_N; i++)
        client_sock[i] = 0;

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
            
            *empty_socket(client_sock) = new_sock;
        }

        for (int i=0; i<CLIENT_N; i++)
        {
            if (client_sock[i] && FD_ISSET(client_sock[i], &sock_set))
            {
                char buf[BUF_SIZE];
                int bytes = recvfrom(client_sock[i], buf, sizeof(buf), 0, NULL, NULL);
                if (bytes <= 0)
                {
                    printf("Client №%d disconnected\n", i);
                    close(client_sock[i]);
                    client_sock[i] = 0;
                } 
                else 
                {
                    buf[bytes] = '\0';
                    printf("Server received: %s\n", buf);
                }
            }
        }
    }

    close_sockets(client_sock, sock);
    return 0;
}