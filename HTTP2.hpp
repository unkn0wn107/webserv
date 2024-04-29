#ifndef HTTP2_H
#define HTTP2_H

#include "HTTPProtocol.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class HTTP2 : public HTTPProtocol {
 public:
  HTTP2();
  virtual ~HTTP2();

  virtual HTTPRequest  parseRequest(const std::string& requestString);
  virtual HTTPResponse createResponse();
  virtual HTTPResponse processRequest(const HTTPRequest& request);
};

#endif
