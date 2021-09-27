#include "header.h"

#define SERV_ADDRESS    "127.0.0.1"

int main(void)
{
    struct hostent *server;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    int msg;

    printf("Input number: ");
    if (!scanf("%d", &msg))
    {
        perror("scaning message failed\n");
        return EXIT_FAILURE;
    }
    *((int *)buf) = msg;

    int sock = socket(SOCKET_TYPE);
    if (sock < 0) 
    {
        perror("socket failed\n");
        return EXIT_FAILURE;
    }

    server = gethostbyname(SERV_ADDRESS);
    if (!server)
    {
        close(sock);
        perror("host not found\n");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *((struct in_addr*) server->h_addr_list[0]);
    serv_addr.sin_port = htons(SERV_PORT);

    printf("Sending message... ");
    if (sendto(sock, &msg, sizeof(int), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        close(sock);
        perror("send failed\n");
        return EXIT_FAILURE;
    }
    printf("done\n");
    
    close(sock);
    return 0;
}