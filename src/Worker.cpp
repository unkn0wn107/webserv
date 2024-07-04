/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 14:24:50 by mchenava          #+#    #+#             */
/*   Updated: 2024/07/02 18:25:25 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"
#include "Common.hpp"
#include "ConfigManager.hpp"

Worker::Worker(Server&                      server,
               int                          epollSocket,
               std::map<int, ListenConfig>& listenSockets)
    : _server(server),
      _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      _epollSocket(epollSocket),
      _listenSockets(listenSockets),
      _load(0) {
  _log.info("Worker constructor called");
}

Worker::~Worker() {}

void Worker::processEvent(struct epoll_event& event) {
  std::clock_t _start = std::clock();

  if (_listenSockets.find(event.data.fd) != _listenSockets.end() && (event.events & EPOLLIN)) {
      _acceptNewConnection(event.data.fd);
  } else {
      ConnectionHandler* handler = static_cast<ConnectionHandler*>(event.data.ptr);
      if (handler && event.events) {
          if (handler->processConnection() == 1) {
            _log.warning("WORKER: Handler deleted with con status " + Utils::to_string(handler->getConnectionStatus()));
            delete handler;
            return;
        }
      }
  }

  std::clock_t end = std::clock();
  double duration = static_cast<double>(end - _start) / CLOCKS_PER_SEC;
  if (duration > 0.005) {
    _log.warning("WORKER: Process event time: " + Utils::to_string(duration) +
                  " seconds");
  }
}

void Worker::_acceptNewConnection(int fd) {
  struct sockaddr_storage address;
  socklen_t               addrlen = sizeof(address);
  int                     new_socket;
  struct epoll_event      event;

  memset(&address, 0, sizeof(address));
  memset(&event, 0, sizeof(event));
  new_socket = accept(fd, (struct sockaddr*)&address, &addrlen); //creer un nouveau sockect d'echange d'event
  if (new_socket <= 0) {
    _log.error("WORKER: New connection: Failed \"accept\"");
    return;
  }
  if (set_non_blocking(new_socket) == -1) {
    _log.error("WORKER: New connection: Failed \"set_non_blocking\"");
    return;
  }
  _log.info("WORKER: New connection: Accepted: " + Utils::to_string(new_socket));
  ListenConfig                listenConfig = _listenSockets[fd];
  std::vector<VirtualServer*> virtualServers = _setupAssociatedVirtualServers(listenConfig);
  ConnectionHandler* handler = new ConnectionHandler(
      new_socket, _epollSocket, virtualServers, listenConfig);
  event.data.ptr = handler;
  event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
  if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, new_socket, &event) < 0) {
    _log.error("WORKER: New connection: Failed \"epoll_ctl\"");
    close(new_socket);
    return;
  }
}

// void Worker::_handleIncomingConnection(epoll_event& event) {
//   ConnectionHandler* handler = static_cast<ConnectionHandler*>(event.data.ptr);
//   _log.info("WORKER (" + Utils::to_string(_threadId) +
//             "): Handling incoming connection at address " +
//             Utils::to_string(reinterpret_cast<uintptr_t>(event.data.ptr)));
//   if (handler->processConnection() == 1)
//     return;
//   delete handler;
//   event.data.ptr = NULL;
//   epoll_ctl(_epollSocket, EPOLL_CTL_DEL, event.data.fd, &event);
// }

std::vector<VirtualServer*> Worker::_setupAssociatedVirtualServers(
    const ListenConfig& listenConfig) {
  std::vector<VirtualServer*> virtualServers;

  for (std::vector<ServerConfig>::const_iterator it = _config.servers.begin();
       it != _config.servers.end(); ++it) {
    for (std::vector<ListenConfig>::const_iterator lit = it->listen.begin();
         lit != it->listen.end(); ++lit) {
      if (*lit == listenConfig) {
        virtualServers.push_back(new VirtualServer(*it));
        break;
      }
    }
  }
  return virtualServers;
}
