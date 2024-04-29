#ifndef ROUTER_H
#define ROUTER_H

#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "Handler.h"
#include <map>
#include <string>

class Router
{
public:
	Router();
	~Router();

	// Route the incoming HTTP request to the appropriate handler based on the URL and method
	Handler *route(const HTTPRequest &request);

	// Add a route to the router with a specific handler for a URL pattern
	void addRoute(const std::string &urlPattern, Handler *handler);

private:
	// Map to store URL patterns and their corresponding handlers
	std::map<std::string, Handler *> routes;
};

#endif // ROUTER_H
