#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ErrorHandler.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"

class Server;  // Forward declaration

class ConnectionHandler {
 public:
  ConnectionHandler(Server& server, int socket);
  ~ConnectionHandler();

  void process();
  void sendResponse();

 private:
  Server&      _server;
  int          _socket;
  HTTPRequest  _request;
  HTTPResponse _response;
  Logger       _logger;
};

#endif
