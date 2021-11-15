#ifndef HEADER_H
#define HEADER_H

#include <sys/socket.h>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE    64
#define SERV_PORT   1830
#define SOCKET_TYPE AF_INET, SOCK_DGRAM, IPPROTO_UDP

#define MAX(a,b) (((a)>(b))?(a):(b))

#endif