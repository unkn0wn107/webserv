/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:02 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:47:54 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() {}

Server::Server(std::map<std::string, std::string> config) : _config(config) {
  start();
}

Server::~Server() {
  close(_epoll_fd);
  close(_server_socket);
}

void Server::start() {
  setupServerSocket();
  setupEpoll();
  run();
}

void Server::setupServerSocket() {
  struct sockaddr_in address;
  int                opt = 1;

  _server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_server_socket == 0)
    ErrorHandler::fatal("Socket creation failed");

  if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)))
    ErrorHandler::fatal("Setsockopt failed");

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(Utils::stoi<int>(_config["port"]));

  if (bind(_server_socket, (struct sockaddr*)&address, sizeof(address)) < 0)
    ErrorHandler::fatal("Bind failed");

  if (listen(_server_socket, 10) < 0)
    ErrorHandler::fatal("Listen failed");

  int flags = fcntl(_server_socket, F_GETFL, 0);
  if (flags == -1 || fcntl(_server_socket, F_SETFL, flags | O_NONBLOCK) == -1)
    ErrorHandler::fatal("Failed to set socket to non-blocking");
}

void Server::setupEpoll() {
  _epoll_fd = epoll_create1(0);
  if (_epoll_fd == -1)
    ErrorHandler::fatal("Epoll creation failed");

  struct epoll_event event;
  event.data.fd = _server_socket;
  event.events = EPOLLIN | EPOLLET | EPOLLOUT;

  if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _server_socket, &event) == -1)
    ErrorHandler::fatal("Epoll control failed");
}

void Server::run() {
  const int          MAX_EVENTS = 10;
  struct epoll_event events[MAX_EVENTS];
  int                event_count;

  while (true) {
    event_count = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
    for (int i = 0; i < event_count; i++) {
      if (events[i].data.fd == _server_socket) {
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

  while ((new_socket = accept(_server_socket, (struct sockaddr*)&address,
                              &addrlen)) != -1) {
    int flags = fcntl(new_socket, F_GETFL, 0);
    if (flags == -1 || fcntl(new_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
      ErrorHandler::log("Failed to set new socket to non-blocking");
      continue;
    }

    ConnectionHandler* handler = new ConnectionHandler(*this, new_socket);
    struct epoll_event event;
    event.data.ptr = handler;
    event.events = EPOLLIN | EPOLLET | EPOLLOUT;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, new_socket, &event) == -1) {
      ErrorHandler::log("Failed to add new socket to epoll");
      delete handler;
    }
  }
}
