#include "common.h"

#define SERV_ADDRESS    "127.0.0.1"


int main(void)
{
    struct hostent *server;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE*2];
    char msg[BUF_SIZE];

    printf("Input message: ");
    if (!scanf("%s", msg))
    {
        perror("scaning message failed\n");
        return EXIT_FAILURE;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
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

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        close(sock);
        perror("connect failed\n");
        return EXIT_FAILURE;
    }


    for (int i=0; i<10; i++)
    {
        sprintf(buf, "Message from %s", msg);
        printf("Sending message... ");
        if (send(sock, buf, strlen(buf), 0) < 0)
        {
            close(sock);
            perror("send failed\n");
            return EXIT_FAILURE;
        }
        printf("done\n");
        
        sleep(2);
        // close(sock);
    }

    close(sock);
    return 0;
}