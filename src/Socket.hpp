#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <string>

class Socket {
 public:
  Socket();
  Socket(const std::string& address, int port);
  ~Socket();

  int         getFd() const;
  std::string getRemoteAddress() const;
  int         getRemotePort() const;
  void        send(const std::string& data);
  std::string receive();

 private:
  int                _fd;
  struct sockaddr_in _address;
};

#endif
