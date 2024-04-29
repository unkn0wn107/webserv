#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include "ConfigLoader.hpp"
#include "ConnectionHandler.hpp"
#include "ErrorHandler.hpp"

class Server {
 public:
  Server();
  Server(std::map<std::string, std::string> config);
  void               start();
  ConnectionHandler* acceptConnection();

 private:
  Server(const Server&);             // Prevent copy-construction
  Server& operator=(const Server&);  // Prevent assignment
  ~Server();

  void setupServer();
  void handleConnections();

  static Server*                  instance;
  ConfigLoader&                   config;
  ErrorHandler                    errorHandler;
  std::vector<ConnectionHandler*> connections;
  int                             epoll_fd;
};

#endif
