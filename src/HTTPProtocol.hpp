#ifndef HTTPPROTOCOL_H
#define HTTPPROTOCOL_H

#include <string>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class HTTPProtocol {
 public:
  HTTPProtocol() {}
  virtual ~HTTPProtocol() {}

  virtual HTTPRequest  parseRequest(const std::string& requestString) = 0;
  virtual HTTPResponse createResponse() = 0;
  virtual HTTPResponse processRequest(const HTTPRequest& request) = 0;
};

#endif
