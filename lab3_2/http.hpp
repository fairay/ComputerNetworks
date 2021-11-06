#include <iostream>
#include <map>

class HttpRequest
{
private:
    size_t _read_headers(const std::string &s_headers);
public:
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers {};
    std::string body;

    HttpRequest(const std::string &content);
    ~HttpRequest();
};

class HttpResponse
{
private:
    std::string head;
public:
    HttpResponse();
    ~HttpResponse();

    int from_file(std::string f_name);

    std::string to_string();
};

