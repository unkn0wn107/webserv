#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <map>
#include "http_request.h"

// Enum for HTTP status codes
enum HttpStatusCode
{
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    INTERNAL_SERVER_ERROR = 500
};

// Class to construct and hold HTTP response details
class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    // Set the status code for the HTTP response
    void setStatusCode(HttpStatusCode statusCode);

    // Set a specific header in the HTTP response
    void setHeader(const std::string &headerName, const std::string &headerValue);

    // Set the body of the HTTP response
    void setBody(const std::string &body);

    // Generate the complete HTTP response as a string
    std::string generateResponse() const;

    // Getters for HTTP response components
    HttpStatusCode getStatusCode() const;
    std::string getHeader(const std::string &headerName) const;
    std::string getBody() const;

private:
    HttpStatusCode statusCode;
    std::map<std::string, std::string> headers;
    std::string body;

    // Helper method to convert HttpStatusCode to string
    std::string statusCodeToString(HttpStatusCode code) const;
};

#endif // HTTP_RESPONSE_H
