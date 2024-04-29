#include "Server.hpp"
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "ConnectionHandler.hpp"

Server::Server() {}

Server::Server(std::map<std::string, std::string> config) : config(config) {
  start();
  ~Server();
}

Server::~Server() {
  close(epoll_fd);
  close(server_socket);
}

void Server::start() {
  setupServerSocket();
  setupEpoll();
  run();
}

void Server::setupServerSocket() {
  struct sockaddr_in address;
  int                opt = 1;

  if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    errorHandler.fatal("Socket creation failed");

  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)))
    errorHandler.fatal("Setsockopt failed");

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(config["port"]);

  if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0)
    errorHandler.fatal("Bind failed");

  if (listen(server_socket, 10) < 0)
    errorHandler.fatal("Listen failed");

  // Set socket to non-blocking mode
  int flags = fcntl(server_socket, F_GETFL, 0);
  if (flags == -1)
    errorHandler.fatal("Failed to get socket flags");

  if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1)
    errorHandler.fatal("Failed to set socket to non-blocking");
}

void Server::setupEpoll() {
  epoll_fd = epoll_create1(0);
  if (epoll_fd == -1)
    errorHandler.fatal("Epoll creation failed");

  struct epoll_event event;
  event.data.fd = server_socket;
  event.events = EPOLLIN | EPOLLET;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1)
    errorHandler.fatal("Epoll control failed");
}

void Server::run() {
  const int          MAX_EVENTS = 10;
  struct epoll_event events[MAX_EVENTS];
  int                event_count;

  while (true) {
    event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    for (int i = 0; i < event_count; i++) {
      if (events[i].data.fd == server_socket) {
        acceptConnection();
      } else {
        ConnectionHandler* handler =
            reinterpret_cast<ConnectionHandler*>(events[i].data.ptr);
        if (events[i].events & EPOLLIN) {
          handler->process();
        }
        if (events[i].events & EPOLLOUT) {
          handler->sendResponse();
        }
      }
    }
  }
}

void Server::acceptConnection() {
  struct sockaddr_in address;
  socklen_t          addrlen = sizeof(address);
  int                new_socket;

  while ((new_socket = accept(server_socket, (struct sockaddr*)&address,
                              &addrlen)) != -1) {
    // Set new socket to non-blocking mode
    int flags = fcntl(new_socket, F_GETFL, 0);
    if (flags == -1) {
      errorHandler.log("Failed to get new socket flags");
      continue;
    }

    if (fcntl(new_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
      errorHandler.log("Failed to set new socket to non-blocking");
      continue;
    }

    ConnectionHandler* handler = new ConnectionHandler(*this, new_socket);
    struct epoll_event event;
    event.data.ptr = handler;
    event.events = EPOLLIN | EPOLLET | EPOLLOUT;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event) == -1) {
      errorHandler.log("Failed to add new socket to epoll");
      delete handler;
    }
  }
}
