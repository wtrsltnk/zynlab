#include "HttpResponse.h"

using namespace net;

Response::Response() : _responseCode(200) { }

Response::~Response() { }

void Response::addHeader(std::string const &key, std::string const &value)
{
    this->_headers.insert(std::make_pair(key, value));
}
