#ifndef HTTP1_1_H
#define HTTP1_1_H

#include <string>
#include "HTTPProtocol.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class HTTP1_1 : public HTTPProtocol {
 public:
  HTTP1_1();
  virtual ~HTTP1_1();

  virtual HTTPRequest  parseRequest(const std::string& requestString);
  virtual HTTPResponse createResponse();
  virtual HTTPResponse processRequest(const HTTPRequest& request);
};

#endif
