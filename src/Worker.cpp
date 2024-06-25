/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 14:24:50 by mchenava          #+#    #+#             */
/*   Updated: 2024/06/10 02:53:21 by agaley           ###   ########.lyon.fr */
/*                                                                            */
/* ************************************************************************** */

#include "Worker.hpp"
#include "Common.hpp"
#include "ConfigManager.hpp"
#include "Server.hpp"

Worker::Worker()
    : _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      _maxConnections(_config.worker_connections),
      _currentConnections(0),
      _shouldStop(false)
{
  _log.info("Worker constructor called");
}

Worker::~Worker() {
  // _stop();

  // pthread_mutex_destroy(&_stopMutex);
}

void  Worker::start() {
  if (pthread_create(&_thread, NULL, _workerRoutine, this) != 0) {
    _log.error("WORKER : Failed to create thread proc");
  }
  pthread_detach(_thread);
}

void Worker::stop() {
  _shouldStop = true;
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
                  it->server_names[0] + "\n");
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
  while (!_shouldStop) {
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
  struct sockaddr_storage address;
  socklen_t              addrlen = sizeof(address);
  int                    new_socket;
  struct epoll_event     event;
  int trys = 0;

  _log.info("WORKER: \"accept\"");
  while (true) {
    new_socket = accept(fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket < 0) {
      if (trys > 3) {
        _log.error("WORKER: Failed \"accept\"");
        break;
      }
      trys++;
      usleep(1000);
      continue;
    }
    trys = 0;
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
  int connectionStatus = handler->getConnectionStatus();
  if (connectionStatus == CLOSED) {
    _log.info("WORKER: Connection closed");
    delete handler;
    return;
  }
  handler->processConnection();
}

void* Worker::_workerRoutine(void* ref) {
  Worker* worker = static_cast<Worker*>(ref);
  worker->_runEventLoop();
  Server::getInstance().workerFinished();
  return NULL;
}
