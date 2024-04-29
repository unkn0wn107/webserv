#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>
#include <vector>

// Enum for HTTP request methods
enum HttpMethod
{
    GET,
    POST,
    DELETE,
    UNKNOWN
};

// Class to parse and hold HTTP request details
class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();

    // Parse the HTTP request from a string buffer
    bool parseRequest(const std::string &requestBuffer);

    // Getters for HTTP request components
    HttpMethod getMethod() const;
    std::string getUri() const;
    std::string getVersion() const;
    std::string getHeader(const std::string &headerName) const;
    std::string getBody() const;

    // Check if the HTTP request is valid
    bool isValid() const;

private:
    HttpMethod method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    bool valid;

    // Helper methods to parse different parts of the HTTP request
    void parseStartLine(const std::string &startLine);
    void parseHeaders(const std::vector<std::string> &headerLines);
    void parseBody(const std::string &bodyContent);

    // Convert method string to HttpMethod enum
    HttpMethod stringToMethod(const std::string &methodStr);
};

#endif // HTTP_REQUEST_H
