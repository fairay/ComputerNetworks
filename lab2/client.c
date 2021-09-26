#include "header.h"

#define handle_error(val, msg) \
    { if (val < 0) \
    { perror(msg); return (EXIT_FAILURE); }}

int main()
{
    char buf[BUF_SIZE];
    int sock;
    struct sockaddr srvr_name;

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket failed\n");
        return EXIT_FAILURE;
    }

    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCK_NAME);

    // strcpy(buf, "Hello, Unix sockets!");
    printf("Input message: ");
    if (!scanf("%s", buf))
    {
        perror("scanf failed\n");
        return EXIT_FAILURE;
    }
    if (sendto(sock, buf, strlen(buf), 0, &srvr_name,
            strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family)) < 0)
    {
        perror("sendto failed\n");
        return EXIT_FAILURE;
    }
    
    printf("Message sent \n");
    close(sock);
    return 0;
}
