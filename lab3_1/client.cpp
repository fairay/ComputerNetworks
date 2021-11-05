#include "common.h"
#include <iostream>

#define SERV_ADDRESS    "127.0.0.1"

class Client
{
private:
    int sock = 0;

    void recv_map();
public:
    Client(std::string address, uint port);
    ~Client();

    void get_file(std::string path, std::string save_path);
};

Client::Client(std::string address, uint port)
{
    struct hostent *server;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        throw std::runtime_error("Error: socket failed");

    server = gethostbyname(address.c_str());
    if (!server)
        throw std::runtime_error("Error: host not found");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *((struct in_addr*) server->h_addr_list[0]);
    serv_addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        throw std::runtime_error("Error: connect failed");
    
    recv_map();
}

Client::~Client()
{
    if (sock)
        close(sock);
}

void Client::recv_map()
{
    const int bsize = 10;
    char buf[bsize+1];
    int bytes;
    do
    {
        bytes = recv(sock, (void*)buf, bsize, 0);
        buf[bytes] = '\0';
        if (bytes <= 0)
            throw std::runtime_error("Error: recv map failed");
        else 
            std::cout << buf;
    } while (bytes == bsize);
}

void Client::get_file(std::string path, std::string save_path="") 
{
    if (send(sock, path.c_str(), path.length(), 0) < 0)
        throw std::runtime_error("Error: send path failed");
    
    file_msg code;
    if (recv(sock, &code, sizeof(code), 0) <= 0)
        throw std::runtime_error("Error: recv code failed");
    else if (code != FILE_OK)
        throw std::runtime_error("Error: file retriving error");
    
    if (save_path == "")
    {
        std::size_t found = path.find_last_of("/\\");
        save_path = path.substr(found+1);
    }

    const int bsize = 200;
    char buf[bsize+1];
    int bytes;
    FILE* fd = fopen(save_path.c_str(), "wb");
    do
    {
        bytes = recv(sock, (void*)buf, bsize, 0);
        buf[bytes] = '\0';
        if (bytes <= 0)
            throw std::runtime_error("Error: recv map failed");
        else 
            printf("%s", buf);
            fwrite(&buf, 1, bytes, fd);
    } while (bytes == bsize);
    fclose(fd);
}

int main(void)
{
    /*
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
    */
   
    Client c(SERV_ADDRESS, SERV_PORT);

    std::string path;
    printf("Input path: ");
    std::cin >> path;
    c.get_file(path);

    /*
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
    */

    return 0;
}