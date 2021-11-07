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

HttpRequest::HttpRequest(): method("GET"), path("/"), version("HTTP/1.1"), body("") {}

HttpRequest::HttpRequest(const std::string &content)
{
    size_t from, to;

    from = 0; to = content.find(" ");
    method = content.substr(from, to-from);

    from = to+1; to = content.find(" ", from);
    path = content.substr(from, to-from);
    path = path.substr(1);
    std::cout << path << std::endl;

    from = to+1; to = content.find("\r", from);
    version = content.substr(from, to-from);

    if (method != "GET")
        throw std::runtime_error("Error: unsupported method of request");
    if (version != "HTTP/1.1")
        throw std::runtime_error("Error: unsupported version of request");
    
    std::cout << "get headers";
    to = _read_headers(content.substr(to+2));
    std::cout << "done \n";
    body = content.substr(to+1);
}

HttpRequest::~HttpRequest()
{
}

size_t HttpRequest::_read_headers(const std::string &s) 
{
    size_t from = 0, to, sep;
    to = s.find("\n", from);
    sep = s.find(":", from);

    if (sep == std::string::npos) return to;
    
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

std::string HttpRequest::to_string()
{
    std::string res;
    res += method + " " + path + " " + version + "\r\n";
    for ( const auto &p : headers )
        res += p.first+": "+p.second+"\r\n";
    res += "\n";
    res += body;
    return res;
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
    head = "HTTP/1.1 ";

    if ((f = fopen(f_name.c_str(), "rb")) == NULL) 
    {
        head += "404 Not Found\r\n";
        code = 404;
    } else 
    {
        head += "200 OK\r\n";
        code = 200;

        struct stat f_info;
        fstat(fileno(f), &f_info);
        size = f_info.st_size;
        fclose(f);

        _add_content_headers(f_name, f_info.st_size);
    }

    head += "Connection: keep-alive\r\n";
    head += "Keep-alive: timeout=5\r\n";
    _add_time_header();
    head += "\n";

    return code;
}

void HttpResponse::_add_content_headers(std::string f_name, size_t size)
{
    std::string ext = f_name.substr(f_name.find_last_of(".") + 1);
    if (ext == "png")
        head += "Content-type: image/png\r\n";
    else
        head += "Content-type: text/html; charset=utf-8\r\n";
    
    char buf[1000];
    sprintf(buf, "Content-length: %ld\r\n", size);
    head += buf;
}

void HttpResponse::_add_time_header()
{
    char buf[100];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, sizeof(buf), "Date: %a, %d %b %Y %H:%M:%S %Z \r\n", &tm);
    head += buf;
}

std::string HttpResponse::to_string()
{
    return head;
}
