#include "http.hpp"
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <ctime>

/*
GET / HTTP/1.1
Host: webkyrs.info
User-Agent: Mozilla/5.0 (Windows NT 6.1; rv:18.0) Gecko/20100101 Firefox/18.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*//*;q=0.8
Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3
Accept-Encoding: gzip, deflate
Cookie: wp-settings
Connection: keep-alive
*/

HttpRequest::HttpRequest(const std::string &content)
{
    size_t from, to;

    from = 0; to = content.find(" ");
    method = content.substr(from, to-from);

    from = to+1; to = content.find(" ", from);
    path = content.substr(from, to-from);
    path = path.substr(1);

    from = to+1; to = content.find("\r", from);
    version = content.substr(from, to-from);

    if (method != "GET")
    {
        std::cout << method;
        throw std::runtime_error("Error: unsupported method of request");
    }
    if (version != "HTTP/1.1")
        throw std::runtime_error("Error: unsupported version of request");
    
    to = _read_headers(content.substr(to+2));
    body = content.substr(to+1);
}

HttpRequest::~HttpRequest()
{
}

size_t HttpRequest::_read_headers(const std::string &s) 
{
    std::cout << s << std::endl;

    size_t from = 0, to, sep;
    to = s.find("\n", from);
    sep = s.find(":", from);
    do
    {
        std::string key, value;
        key = s.substr(from, sep-from);
        value = s.substr(sep+2, to-sep-3);

        headers[key] = value;
        from = to + 1;
        to = s.find("\n", from);
        sep = s.find(":", from);
    } while (from != to - 1);
    return to;
}


HttpResponse::HttpResponse(/* args */)
{
}

HttpResponse::~HttpResponse()
{
}

int HttpResponse::from_file(std::string f_name) 
{   
    FILE *f;
    int code = -1;
    size_t size = 0;
    this->head += "HTTP/1.0 ";

    if ((f = fopen(f_name.c_str(), "rb")) == NULL) 
    {
        this->head += "404 Not Found\r\n";
        code = 404;
    } else {
        this->head += "200 OK\r\n";
        code = 200;

        struct stat f_info;
        fstat(fileno(f), &f_info);
        size = f_info.st_size;
        fclose(f);
    }

    head += "Connection: keep-alive\r\n";
    head += "Keep-alive: timeout=5\r\n";

    std::string ext = f_name.substr(f_name.find_last_of(".") + 1);
    if (ext == "png")
        head += "Content-type: image/png\r\n";
    else
        head += "Content-type: text/html; charset=utf-8\r\n";
    // head += "Content-length: 0\r\n";

    char buf[1000];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    head += "Date: " + std::string(buf) + "\r\n";

    head += "\n";

    return code;
}

std::string HttpResponse::to_string()
{
    return head;
}
