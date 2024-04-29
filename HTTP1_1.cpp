#include "HTTP1_1.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include <sstream>
#include <algorithm>

HTTP1_1::HTTP1_1() {}

HTTPRequest HTTP1_1::parseRequest(const std::string& requestData) {
    HTTPRequest request;
    std::istringstream stream(requestData);
    std::string line;
    std::getline(stream, line);

    // Parse the request line
    std::istringstream requestLine(line);
    std::string method;
    std::string url;
    std::string version;
    requestLine >> method >> url >> version;

    request.setMethod(method);
    request.setUrl(url);

    // Parse headers
    while (std::getline(stream, line) && line != "\r") {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) continue;
        std::string headerName = line.substr(0, line.find(":"));
        std::string headerValue = line.substr(line.find(":") + 2);
        request.addHeader(headerName, headerValue);
    }

    // Parse body if any
    std::string body;
    if (request.getHeader("Content-Length") != "") {
        int contentLength = std::stoi(request.getHeader("Content-Length"));
        char* buffer = new char[contentLength + 1];
        stream.read(buffer, contentLength);
        buffer[contentLength] = '\0';
        body = std::string(buffer, contentLength);
        delete[] buffer;
    }
    request.setBody(body);

    return request;
}

HTTPResponse HTTP1_1::createResponse() {
    return HTTPResponse();
}

HTTPResponse HTTP1_1::processRequest(const HTTPRequest& request) {
    HTTPResponse response = createResponse();
    response.setStatusCode(200); // OK by default
    response.setProtocol("HTTP/1.1");

    // Example processing logic
    if (request.getMethod() == "GET") {
        response.setBody("Received a GET request for " + request.getUrl());
    } else if (request.getMethod() == "POST") {
        response.setBody("Received a POST request with body: " + request.getBody());
    } else {
        response.setStatusCode(405); // Method Not Allowed
        response.setBody("Method Not Supported");
    }

    // Set common headers
    response.addHeader("Content-Type", "text/plain");
    response.addHeader("Content-Length", std::to_string(response.getBody().length()));

    return response;
}
