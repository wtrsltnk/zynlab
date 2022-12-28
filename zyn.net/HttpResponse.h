#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>

#include "HttpRequest.h"

namespace net
{

    class Response
    {
    public:
        Response();
        virtual ~Response();

        void addHeader(
            std::string const &key,
            std::string const &value);

        int _responseCode;
        std::map<std::string, std::string> _headers;
        std::string _response;
    };

} // namespace net

#endif // HTTPRESPONSE_H
