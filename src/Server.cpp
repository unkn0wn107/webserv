/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:02 by agaley            #+#    #+#             */
/*   Updated: 2024/05/07 09:16:32 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Config.hpp"
#include "Logger.hpp"

Server::Server(ServerConfig& config)
    : _config(config), _log(Logger::getInstance()) {
  _log.info("Server " + _config.server_names[0] + "is starting");
  _log.info("Listening on port " + _config.listen_port);
  setupServerSocket();
  setupEpoll();
}

Server::~Server() {
  closeAllClients();
  close(_epoll_fd);
  close(_server_socket);
}

void Server::setupServerSocket() {
  struct sockaddr_in6 address;
  int                 opt = 1;
  int                 ipv6only = 0;

  _server_socket = socket(AF_INET6, SOCK_STREAM, 0);
  if (_server_socket == 0)
    ErrorHandler::fatal("Socket creation failed");

  // reuseport si definit par la config
  // https://nginx.org/en/docs/http/ngx_http_core_module.html#reuseport
  if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)))
    ErrorHandler::fatal("Setsockopt failed");

  if (setsockopt(_server_socket, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6only,
                 sizeof(ipv6only)))
    ErrorHandler::fatal("Setsockopt IPV6_V6ONLY failed");

  address.sin6_family = AF_INET6;
  address.sin6_addr = in6addr_any;
  address.sin6_port = htons(_config.listen_port);

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
      _log.error("(" + _config.server_names[0] +
                 ") Failed to set new socket to non-blocking");
      continue;
    }

    _client_sockets.insert(new_socket);

    ConnectionHandler* handler = new ConnectionHandler(new_socket, _config);
    struct epoll_event event;
    event.data.ptr = handler;
    event.events = EPOLLIN | EPOLLET | EPOLLOUT;

    if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, new_socket, &event) == -1) {
      ErrorHandler::log("Failed to add new socket to epoll");
      _log.error("(" + _config.server_names[0] +
                 ") Failed to add new socket to epoll");
      delete handler;
      close(new_socket);
      _client_sockets.erase(new_socket);
    }
  }
}

void Server::closeAllClients() {
  for (std::set<int>::iterator it = _client_sockets.begin();
       it != _client_sockets.end(); ++it) {
    close(*it);
  }
  _client_sockets.clear();
}
