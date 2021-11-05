#ifndef HEADER_H
#define HEADER_H

#include <sys/socket.h>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>

#define BUF_SIZE    1024
#define SERV_PORT   1831

enum file_msg
{
    FILE_OK,
    NOT_FOUND
};

#define MAX(a,b) (((a)>(b))?(a):(b))

#endif
