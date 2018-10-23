#include "HttpClient.h"

#include <sstream>
#include <string>
#include <windows.h>
#include <wininet.h>

using namespace net;

HttpClient::~HttpClient() {}

bool HttpClientRequest(std::string const &site, std::string const &param, std::string const &method, std::string &response)
{
    HINTERNET hInternet = InternetOpen("YourUserAgent", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0); //you should perhaps edit your useragent ? :p

    if (hInternet == nullptr)
    {
        return false;
    }

    HINTERNET hConnect = InternetConnect(hInternet, site.c_str(), 80, nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);

    if (hConnect == nullptr)
    {
        InternetCloseHandle(hInternet);

        return false;
    }

    char const *parrAcceptTypes[] = {"text/*", nullptr}; // accepted types. We'll choose text.

    HINTERNET hRequest = HttpOpenRequest(hConnect, method.c_str(), param.c_str(), nullptr, nullptr, parrAcceptTypes, 0, 0);

    if (hRequest == nullptr)
    {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        return false;
    }

    BOOL bRequestSent = HttpSendRequest(hRequest, nullptr, 0, nullptr, 0);

    if (!bRequestSent)
    {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        return false;
    }

    std::string strResponse;
    const int nBuffSize = 1024;
    char buff[nBuffSize];

    BOOL bKeepReading = true;
    DWORD dwBytesRead = 0;

    while (bKeepReading)
    {
        bKeepReading = InternetReadFile(hRequest, buff, nBuffSize, &dwBytesRead);

        strResponse.append(buff, dwBytesRead);

        if (dwBytesRead == 0)
        {
            break;
        }
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    response = strResponse;

    return true;
}

std::string HttpClient::get(std::string const &url)
{
    std::string response;

    if (HttpClientRequest(url, "", "GET", response))
    {
        return response;
    }

    return "";
}

std::string HttpClient::post(std::string const &url, std::map<std::string, std::string> const &data)
{
    std::string response;
    std::stringstream ss;

    for (auto pair : data)
    {
        // can we assume the data is already encoded?
        ss << "&" << pair.first << "=" << pair.second;
    }

    std::string dataAsString = ss.str().substr(1);

    if (HttpClientRequest(url, dataAsString, "POST", response))
    {
        return response;
    }

    return "";
}
