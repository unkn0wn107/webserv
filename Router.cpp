#include "Router.hpp"
#include <stdexcept>
#include "CGIHandler.hpp"
#include "FileHandler.hpp"

Router::Router() {
  // Initialize any necessary components or routes here
}

Handler* Router::route(const HTTPRequest& request) {
  // Determine the appropriate handler based on the request URL and method
  std::string url = request.getUrl();
  std::string method = request.getMethod();

  // Example routing logic
  // This should be replaced with actual routing logic based on the server
  // configuration
  if (url.find("/cgi-bin/") == 0) {
    // If the URL indicates a CGI script, use the CGIHandler
    return new CGIHandler();
  } else {
    // Otherwise, use the FileHandler for static files
    return new FileHandler();
  }

  // If no appropriate handler is found, throw an exception
  throw std::runtime_error("No suitable handler found for the request.");
}
