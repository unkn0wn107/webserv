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

Worker::Worker(int epollSocket, std::map<int, ListenConfig>& listenSockets)
    : _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      // _maxConnections(_config.worker_connections),
      // _currentConnections(0),
      _epollSocket(epollSocket),
      _listenSockets(listenSockets),
      _events(),
      _load(0), 
      _shouldStop(false)
{
  _log.info("Worker constructor called");
  pthread_mutex_init(&_queueMutex, NULL);
}

Worker::~Worker() {
  pthread_mutex_destroy(&_queueMutex);
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
    _log.error(std::string("WORKER (" + Utils::to_string(_thread) + "): Failed \"epoll_create1\": ") +
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
        _log.info("WORKER (" + Utils::to_string(_thread) + "): Associate VirtualServer to a conn: " +
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
  _load++;
  _log.info("WORKER (" + Utils::to_string(_thread) + "): Pushed event to queue : " + Utils::to_string(event.data.fd));
}

int Worker::getLoad() {
  return _load;
}

void Worker::_runEventLoop() {
  while (!_shouldStop) {
    if (_load == 0) {
      usleep(1000);
      continue;
    }
    pthread_mutex_lock(&_queueMutex);
    if (_events.empty()) {
      pthread_mutex_unlock(&_queueMutex);
      continue;
    }
    struct epoll_event event = _events.front();
    _events.pop();
    pthread_mutex_unlock(&_queueMutex);
    _load--;
    _log.info("WORKER (" + Utils::to_string(_thread) + "): Event loop: " + Utils::to_string(event.data.fd));
    if (_listenSockets.find(event.data.fd) != _listenSockets.end() && event.events & EPOLLIN)
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

  _log.info("WORKER (" + Utils::to_string(_thread) + "): \"accept\" fd :" + Utils::to_string(fd));
  while (true) {
    new_socket = accept(fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket < 0) {
      _log.error("WORKER (" + Utils::to_string(_thread) + "): Failed \"accept\" " + strerror(errno));
      if (trys > 10) {
        _log.error("WORKER (" + Utils::to_string(_thread) + "): Failed \"accept\"");
        break;
      }
      trys++;
      usleep(1000);
      continue;
    }
    trys = 0;
    if (set_non_blocking(new_socket) == -1) {
      _log.error("WORKER (" + Utils::to_string(_thread) + "): Failed \"set_non_blocking\"");
      continue;
    }
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    _log.info("WORKER (" + Utils::to_string(_thread) + "): Accepted connection : " + Utils::to_string(fd) + " -> " + Utils::to_string(new_socket));
    ListenConfig listenConfig = _listenSockets[fd];
    std::vector<VirtualServer*> virtualServers = _setupAssociateVirtualServer(listenConfig);
    ConnectionHandler* handler = new ConnectionHandler(
        new_socket, _epollSocket, virtualServers);
    _handlers[new_socket] = handler;
    event.data.ptr = handler;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, new_socket, &event) < 0) {
      _log.error("WORKER (" + Utils::to_string(_thread) + "): Failed \"epoll_ctl\"");
      continue;
    }
    break;
  }
}

void Worker::_handleIncomingConnection(struct epoll_event event) {
  _log.info("WORKER (" + Utils::to_string(_thread) + "): Handling incoming connection for fd " + Utils::to_string(event.data.fd));
  ConnectionHandler* handler = static_cast<ConnectionHandler*>(event.data.ptr);
  int connectionStatus = handler->getConnectionStatus();
  if (connectionStatus == CLOSED) {  
    _log.info("WORKER (" + Utils::to_string(_thread) + "): Connection closed");
    delete handler;
    return;
  }
  handler->processConnection();
}

void* Worker::_workerRoutine(void* ref) {
  Worker* worker = static_cast<Worker*>(ref);
  Logger::getInstance().info("WORKER (" + Utils::to_string(worker->_thread) + "): Worker routine started");
  worker->_runEventLoop();
  Server::getInstance().workerFinished();
  return NULL;
}
