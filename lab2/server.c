#include "header.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

#define DIGIT_STR   "0123456789ABCDEF"
const int maxlen = 101;
int sock;

void sigint_action(int signum)
{
    printf("\nInterrupt signal received, closing server\n");
    close(sock);
    exit(0);
}

int to_n_base(int number, int base, char* dstr) 
{
    char sstr[maxlen];

    if (number < 0) 
    {
        *(dstr++) = '-';
        number = -number;
    }

    int i = maxlen-2;
    sstr[i+1] = '\0';
    if (!number) sstr[i--] = DIGIT_STR[0];
    while (number)
    {
        int rem = number % base;
        number /= base;
        sstr[i--] = DIGIT_STR[rem];
    }

    strcpy(dstr, sstr+i+1);
    return 0;
}

void print_all_base(int num) 
{
    char buf[maxlen];

    const int base_n = 5;
    int base_arr[] = { 10, 2, 16, 8, 9 };
    for (int i=0; i<base_n; i++) 
    {
        to_n_base(num, base_arr[i], buf);
        printf("%d (base %d) = %s\n", num, base_arr[i], buf);
    }
}

int main(void)
{
    struct sockaddr_in serv_addr;
    
    sock = socket(SOCKET_TYPE);
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
        int num;
        if (recvfrom(sock, &num, sizeof(num), 0, NULL, NULL) != sizeof(num))
        {
            printf("Error");
            break;
        } 
        print_all_base(num);
    }

    close(sock);
    return 0;
}