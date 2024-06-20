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

Worker::Worker(int epollSocket, std::set<t_listen_socket>& listenSockets)
    : _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      // _maxConnections(_config.worker_connections),
      // _currentConnections(0),
      _epollSocket(epollSocket),
      _listenSockets(listenSockets),
      _events(),
      _shouldStop(false)
{
  _log.info("Worker constructor called");
  pthread_mutex_init(&_queueMutex, NULL);
  _events.push(epoll_event());
  _events.pop();
  _log.info("WORKER: event queue size: " + Utils::to_string(_events.size()));
}

Worker::~Worker() {
  // _stop();
  pthread_mutex_destroy(&_queueMutex);

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
                  it->server_names[0] + "\n");
        virtualServers.push_back(new VirtualServer(*it));
        break;
      }
    }
  }
  return virtualServers;
}


void Worker::pushEvent(struct epoll_event event) {
  pthread_mutex_lock(&_queueMutex);
  _events.push(event);
  pthread_mutex_unlock(&_queueMutex);
  _log.info("WORKER: Pushed event to queue");
}

int Worker::getLoad() {
  pthread_mutex_lock(&_queueMutex);
  int load = _events.size();
  pthread_mutex_unlock(&_queueMutex);
  return load;
}

void Worker::_runEventLoop() {
  while (!_shouldStop) {
    pthread_mutex_lock(&_queueMutex);
    if (_events.empty()) {
      usleep(1000);
      pthread_mutex_unlock(&_queueMutex);
      continue;
    }
    pthread_mutex_unlock(&_queueMutex);
    struct epoll_event event = _events.front();
    _events.pop();
    pthread_mutex_unlock(&_queueMutex);
    if (isInSet(event.data.fd, _listenSockets))
      _acceptNewConnection(event.data.fd);
    else
      _handleIncomingConnection(event);
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
    std::vector<VirtualServer*> virtualServers = Server::getVirtualServer(fd);
    ConnectionHandler* handler = new ConnectionHandler(
        new_socket, _epollSocket, virtualServers);
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
