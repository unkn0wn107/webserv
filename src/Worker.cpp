/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 14:24:50 by mchenava          #+#    #+#             */
/*   Updated: 2024/06/04 14:43:23 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"
#include <algorithm>
#include "Common.hpp"
#include "ConfigManager.hpp"

Worker::Worker()
    : _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      _maxConnections(_config.worker_connections),
      _currentConnections(0) {
  _log.info("Worker constructor called");
  _setupEpoll();
  if (pthread_create(&_thread, NULL, _workerRoutine, this) != 0) {
    _log.error("WORKER : Failed to create thread proc");
  }
}

Worker::~Worker() {
  pthread_join(_thread, NULL);
}

void Worker::_setupEpoll() {
  _epollSocket = epoll_create1(0);
  if (_epollSocket <= 0) {
    _log.error(std::string("WORKER: Failed \"epoll_create1\": ") +
               strerror(errno));
    exit(EXIT_FAILURE);
  }
}

std::vector<VirtualServer*> Worker::_setupAssociateVirtualServer(
    const ListenConfig& listenConfig) {
  std::vector<VirtualServer*> virtualServers;

  for (std::vector<ServerConfig>::iterator it = _config.servers.begin();
       it != _config.servers.end(); ++it) {
    for (std::vector<ListenConfig>::iterator lit = it->listen.begin();
         lit != it->listen.end(); ++lit) {
      if (*lit == listenConfig) {
        _log.info("WORKER: Associate VirtualServer to a conn: " +
                  it->server_names[0]);
        virtualServers.push_back(new VirtualServer(*it));
        break;
      }
    }
  }
  return virtualServers;
}

void Worker::assignConnection(int                 clientSocket,
                              const ListenConfig& listenConfig) {
  struct epoll_event event;
  event.data.fd = clientSocket;
  event.events = EPOLLIN | EPOLLET;

  if (_currentConnections >= _maxConnections) {
    _log.error("WORKER: Max connections reached");
    close(clientSocket);
  } else {
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
      close(clientSocket);
      _log.error(std::string("WORKER (assign conn): Failed \"epoll_ctl\": ") +
                 strerror(errno) + " (" + Utils::to_string(clientSocket) + ")");
      return;
    }
    _listenSockets.push_back(clientSocket);
    _listenConfigs[clientSocket] = listenConfig;
    _virtualServers[clientSocket] = _setupAssociateVirtualServer(listenConfig);
    _currentConnections++;
  }
}

void Worker::_runEventLoop() {
  struct epoll_event events[MAX_EVENTS];
  int                nfds;

  while (true) {
    nfds = epoll_wait(_epollSocket, events, MAX_EVENTS, -1);
    if (nfds <= 0) {
      _log.error("Erreur lors de l'attente des événements epoll");
      continue;
    }

    for (int n = 0; n < nfds; ++n) {
      if (std::find(_listenSockets.begin(), _listenSockets.end(),
                    events[n].data.fd) != _listenSockets.end())
        _acceptNewConnection(events[n].data.fd);
      else
        _handleIncomingConnection(events[n]);
    }
  }
}

void Worker::_acceptNewConnection(int fd) {
  struct sockaddr    address;
  socklen_t          addrlen = sizeof(address);
  int                new_socket;
  struct epoll_event event;

  while (true) {
    new_socket = accept(fd, &address, &addrlen);
    if (new_socket < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
        break;
      else {
        _log.error("WORKER: Failed \"accept\"");
        break;
      }
    }
    if (set_non_blocking(new_socket) == -1) {
      _log.error("WORKER: Failed \"set_non_blocking\"");
      continue;
    }
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ConnectionHandler* handler = new ConnectionHandler(
        new_socket, _epollSocket, /*_listenConfigs[fd],*/ _virtualServers[fd]);
    _handlers[new_socket] = handler;
    event.data.ptr = handler;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, new_socket, &event) < 0) {
      _log.error("WORKER (new conn): Failed \"epoll_ctl\"");
      continue;
    }
  }
}

void Worker::_handleIncomingConnection(struct epoll_event& event) {
  _log.info("WORKER: Handling incoming connection");
  ConnectionHandler* handler = (ConnectionHandler*)event.data.ptr;
  handler->processConnection();
}

void* Worker::_workerRoutine(void* ref) {
  Worker* worker = static_cast<Worker*>(ref);
  worker->_runEventLoop();
  return NULL;
}
