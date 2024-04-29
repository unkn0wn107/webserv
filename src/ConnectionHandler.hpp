#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <map>
#include <string>
#include "ErrorHandler.hpp"
#include "FileManager.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "Router.hpp"

class ConnectionHandler {
 public:
  ConnectionHandler();
  ConnectionHandler(std::map<std::string, std::string> config);
  ConnectionHandler(const ConnectionHandler&);
  ConnectionHandler& operator=(const ConnectionHandler&);
  ~ConnectionHandler();

  void handleConnection(int socket);

 private:
  int            _socket;
  HTTPRequest  _request;
  HTTPResponse _response;
  Router       _router;
  Logger       _logger;
};

#endif
