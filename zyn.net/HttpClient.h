#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>
#include <map>

namespace net
{

class HttpClient
{
public:
    virtual ~HttpClient();

    std::string get(std::string const &url);
    std::string post(std::string const &url, std::map<std::string,std::string> const &data);
};

}

#endif // HTTPCLIENT_H
