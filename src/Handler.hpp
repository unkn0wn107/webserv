#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class Handler {
 public:
  virtual ~Handler() {}
  virtual HTTPResponse handle(const HTTPRequest& request) = 0;
};

#endif
