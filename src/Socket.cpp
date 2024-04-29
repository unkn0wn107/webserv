#include "Socket.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

Socket::Socket() {
  _fd = socket(AF_INET, SOCK_STREAM, 0);
  if (_fd == -1) {
    throw std::runtime_error("Failed to create socket");
  }
}

Socket::Socket(const std::string& address, int port) {
  _fd = socket(AF_INET, SOCK_STREAM, 0);
  if (_fd == -1) {
    throw std::runtime_error("Failed to create socket");
  }

  memset(&_address, 0, sizeof(_address));
  _address.sin_family = AF_INET;
  _address.sin_port = htons(port);

  if (inet_pton(AF_INET, address.c_str(), &_address.sin_addr) <= 0) {
    throw std::runtime_error("Failed to convert address");
  }

  if (connect(_fd, (struct sockaddr*)&_address, sizeof(_address)) == -1) {
    throw std::runtime_error("Failed to connect to remote address");
  }
}

Socket::~Socket() {
  close(_fd);
}

int Socket::getFd() const {
  return _fd;
}

std::string Socket::getRemoteAddress() const {
  char buffer[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &_address.sin_addr, buffer, INET_ADDRSTRLEN);
  return std::string(buffer);
}

int Socket::getRemotePort() const {
  return ntohs(_address.sin_port);
}

void Socket::send(const std::string& data) {
  if (send(_fd, data.c_str(), data.size(), 0) == -1) {
    throw std::runtime_error("Failed to send data");
  }
}

std::string Socket::receive() {
  char    buffer[1024];
  ssize_t bytes_received = recv(_fd, buffer, sizeof(buffer), 0);
  if (bytes_received <= 0) {
    throw std::runtime_error("Failed to receive data");
  }
  return std::string(buffer, bytes_received);
}
