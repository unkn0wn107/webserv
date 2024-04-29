#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <vector>
#include "ErrorHandler.hpp"

class ConnectionHandler {
 public:
  ConnectionHandler();
  ConnectionHandler(std::map<std::string, std::string> config);
  ConnectionHandler(const ConnectionHandler&);
  ConnectionHandler& operator=(const ConnectionHandler&);
  ~ConnectionHandler();

 private:
};

#endif
