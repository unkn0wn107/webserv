#ifndef HTTPPROTOCOL_H
#define HTTPPROTOCOL_H

#include <string>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

// Abstract class for HTTP Protocol handling
class HTTPProtocol {
 public:
  // Constructor
  HTTPProtocol() {}

  // Virtual destructor for proper cleanup of derived classes
  virtual ~HTTPProtocol() {}

  // Pure virtual function to parse the incoming request string and return an
  // HTTPRequest object
  virtual HTTPRequest parseRequest(const std::string& requestString) = 0;

  // Pure virtual function to create an HTTPResponse object
  virtual HTTPResponse createResponse() = 0;

  // Pure virtual function to process the HTTPRequest and generate an
  // HTTPResponse
  virtual HTTPResponse processRequest(const HTTPRequest& request) = 0;
};

#endif  // HTTPPROTOCOL_H
