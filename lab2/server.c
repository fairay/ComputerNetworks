#include "header.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#define DIGIT_STR   "0123456789ABCDEF"

void sigint_action(int signum)
{
    printf("\nInterrupt signal received, closing server\n");
    exit(0);
}

int to_n_base(int number, int base, char* dstr) 
{
    const int maxlen = 101;
    char sstr[maxlen];
    memset(sstr, 0, maxlen);

    int is_neg = 0;
    if (number < 0) 
    {
        is_neg = 1;
        number = -number;
    }

    int i = maxlen-2;
    if (!number) sstr[i--] = DIGIT_STR[0];
    while (number)
    {
        int rem = number % base;
        number /= base;
        sstr[i--] = DIGIT_STR[rem];
    }
    if (is_neg) sstr[i--] = '-';

    strcpy(dstr, sstr+i+1);
    return 0;
}

int main(void)
{
    struct sockaddr_in serv_addr;
    
    int sock = socket(SOCKET_TYPE);
    if (sock < 0) 
    {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        close(sock);
        perror("bind failed\n");
        return EXIT_FAILURE;
    }

    printf("Socket linked, server listening\n");
    signal(SIGINT, sigint_action);
    while (1)
    {
        char buf[BUF_SIZE];
        int bytes = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
        if (bytes <= 0)
        {
            printf("Error");
            break;
        } 
        buf[bytes] = '\0';
        printf("Server received: %s\n", buf);

        int num;
        sscanf(buf, "%d", &num);
        to_n_base(num, 3, buf);
        printf("3 base %d = %s\n", num, buf);
    }

    close(sock);
    return 0;
}