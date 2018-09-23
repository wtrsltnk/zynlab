#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <winsock2.h>
#include <functional>
#include <string>
#include <map>

#define BUFFER_SIZE 1024*5 // 5KB

namespace net
{

class Request
{
    SOCKET _socket;
    sockaddr_in _clientInfo;
    std::string getMessage();
public:
    Request(SOCKET socket, sockaddr_in clientInfo);

    static void handleRequest(std::function<int (const Request &, class Response &)> onConnection, Request request);

    std::string _method;
    std::string _uri;
    std::map<std::string, std::string> _headers;
    std::string _payload;

    std::string ipAddress() const;
};

class RequestHandler
{
public:
    virtual ~RequestHandler() { }

    virtual int ConstructResponse(const Request& request, class Response& response) const = 0;
};

}

#endif // HTTPREQUEST_H
