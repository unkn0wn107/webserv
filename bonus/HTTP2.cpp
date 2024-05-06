#include "HTTP2.hpp"
#include <iostream>
#include <sstream>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

HTTP2::HTTP2() {
  // Constructor for HTTP/2 protocol handler
}

HTTPRequest HTTP2::parseRequest(const std::string& requestData) {
  HTTPRequest        request;
  std::istringstream stream(requestData);
  std::string        line;
  std::getline(stream, line);  // Get the request line

  // Parse the request line
  size_t methodEnd = line.find(' ');
  if (methodEnd != std::string::npos) {
    request.setMethod(line.substr(0, methodEnd));
    size_t pathStart = methodEnd + 1;
    size_t pathEnd = line.find(' ', pathStart);
    if (pathEnd != std::string::npos) {
      request.setUrl(line.substr(pathStart, pathEnd - pathStart));
    }
  }

  // Parse headers
  while (std::getline(stream, line) && line != "\r") {
    size_t separator = line.find(':');
    if (separator != std::string::npos) {
      std::string key = line.substr(0, separator);
      std::string value = line.substr(
          separator + 2,
          line.length() - separator - 3);  // Skip colon and space, and trim \r
      request.addHeader(key, value);
    }
  }

  // Parse body if present
  if (request.getMethod() == "POST" || request.getMethod() == "PUT") {
    std::string body;
    while (std::getline(stream, line)) {
      body += line + "\n";
    }
    request.setBody(body);
  }

  return request;
}

HTTPResponse HTTP2::createResponse() {
  HTTPResponse response;
  // Default settings for HTTP/2 response
  response.setStatusCode(200);  // OK by default
  response.setProtocol("HTTP/2");
  return response;
}

HTTPResponse HTTP2::processRequest(const HTTPRequest& request) {
  HTTPResponse response = createResponse();

  // Example processing logic
  if (request.getUrl() == "/") {
    response.setBody("Welcome to HTTP/2 Server!");
  } else {
    response.setStatusCode(404);  // Not Found
    response.setBody("404 Not Found");
  }

  // Set necessary HTTP/2 headers
  response.addHeader("Content-Type", "text/html");
  response.addHeader("Content-Length",
                     std::to_string(response.getBody().length()));

  return response;
}
