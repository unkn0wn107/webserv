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

Worker::Worker(Server& server, int epollSocket, std::map<int, ListenConfig>& listenSockets)
    : _server(server),
      _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      _epollSocket(epollSocket),
      _listenSockets(listenSockets),
      _load(0),
      _shouldStop(false)
{
}

Worker::~Worker() {
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
        virtualServers.push_back(new VirtualServer(*it));
        break;
      }
    }
  }
  return virtualServers;
}

int Worker::getLoad() {
  return _load;
}

void Worker::_runEventLoop() {
  while (!_shouldStop) {
    struct epoll_event event = _server.getEvent();
    if (event.data.fd == 0) {
      usleep(1000);
      continue;
    }
    if (_listenSockets.find(event.data.fd) != _listenSockets.end() && event.events & EPOLLIN)
      _acceptNewConnection(event.data.fd);
    else
      _handleIncomingConnection(event);
  }
  _log.info("WORKER (" + Utils::to_string(_threadId) + "): End of event loop");
}

void Worker::_acceptNewConnection(int fd) {
  struct sockaddr_storage address;
  socklen_t              addrlen = sizeof(address);
  int                    new_socket;
  struct epoll_event     event;
  while (!_shouldStop) {
    new_socket = accept(fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket <= 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }
      _log.info("WORKER (" + Utils::to_string(_thread) + "): Failed \"accept\" for fd =" + Utils::to_string(fd) + ": " + strerror(errno));
      break;
    }
    if (set_non_blocking(new_socket) == -1) {
      _log.error("WORKER (" + Utils::to_string(_thread) + "): Failed \"set_non_blocking\"");
      continue;
    }
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ListenConfig listenConfig = _listenSockets[fd];
    std::vector<VirtualServer*> virtualServers = _setupAssociateVirtualServer(listenConfig);
    ConnectionHandler* handler = new ConnectionHandler(
        new_socket, _epollSocket, virtualServers);
    event.data.ptr = handler;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, new_socket, &event) < 0) {
      _log.error("WORKER (" + Utils::to_string(_thread) + "): Failed \"epoll_ctl\"");
      continue;
    }
  }
}

void Worker::_handleIncomingConnection(struct epoll_event event) {
  ConnectionHandler* handler = static_cast<ConnectionHandler*>(event.data.ptr);
  handler->processConnection();
  epoll_ctl(_epollSocket, EPOLL_CTL_DEL, event.data.fd, &event);
}

void* Worker::_workerRoutine(void* ref) {
  Worker* worker = static_cast<Worker*>(ref);
  worker->_threadId = gettid();
  worker->_log.info("WORKER (" + Utils::to_string(worker->_threadId) + "): Started");
  worker->_runEventLoop();
  worker->_log.info("WORKER (" + Utils::to_string(worker->_threadId) + "): Finished");
  worker->_server.workerFinished();
  return NULL;
}
