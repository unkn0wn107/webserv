#ifndef ROUTER_H
#define ROUTER_H

#include <map>
#include <string>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Handler.hpp"

class Router {
 public:
  Router();
  ~Router();

  Handler* route(const HTTPRequest& request);

  // Add a route to the router with a specific handler for a URL pattern
  void addRoute(const std::string& urlPattern, Handler* handler);

 private:
  // Map to store URL patterns and their corresponding handlers
  std::map<std::string, Handler*> routes;
};

#endif
