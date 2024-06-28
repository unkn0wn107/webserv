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

Worker::Worker(Server&                      server,
               int                          epollSocket,
               std::map<int, ListenConfig>& listenSockets)
    : _server(server),
      _thread(0),
      _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      _epollSocket(epollSocket),
      _listenSockets(listenSockets),
      _load(0),
      _shouldStop(false)
{
  _log.info("Worker constructor called");
}

Worker::~Worker() {}

void Worker::start() {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  if (pthread_create(&_thread, &attr, _workerRoutine, this) != 0) {
    _log.error("WORKER : Failed to create thread proc");
  }

  pthread_attr_destroy(&attr);
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
  struct epoll_event events[MAX_EVENTS];
  while (!_shouldStop) {
    int nfds = epoll_wait(_epollSocket, events, MAX_EVENTS, -1);
    if (nfds <= 0) {
      usleep(1000);
      continue;
    }
    for (int i = 0; i < nfds && !_shouldStop; i++) {
      _log.info("WORKER (" + Utils::to_string(_threadId) +
                "): Event loop: " + Utils::to_string(events[i].data.fd));
      if (_listenSockets.find(events[i].data.fd) != _listenSockets.end())
        _acceptNewConnection(events[i].data.fd);
      else
        _handleIncomingConnection(events[i]);
    }
  }
  _log.info("WORKER (" + Utils::to_string(_threadId) + "): End of event loop");
}

void Worker::_acceptNewConnection(int fd) {
  struct sockaddr_storage address;
  socklen_t               addrlen = sizeof(address);
  int                     new_socket;
  struct epoll_event      event;

  while (!_shouldStop) {
    new_socket = accept(fd, (struct sockaddr*)&address, &addrlen);
    if (new_socket <= 0) {
      break;
    }
    if (set_non_blocking(new_socket) == -1) {
      _log.error("WORKER (" + Utils::to_string(_thread) +
                 "): Failed \"set_non_blocking\"");
      continue;
    }
    _log.info("WORKER (" + Utils::to_string(_thread) +
              "): Accepted new connection: " + Utils::to_string(new_socket));
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    ListenConfig                listenConfig = _listenSockets[fd];
    std::vector<VirtualServer*> virtualServers =
        _setupAssociateVirtualServer(listenConfig);
    ConnectionHandler* handler =
        new ConnectionHandler(new_socket, _epollSocket, virtualServers, listenConfig);
    event.data.ptr = handler;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, new_socket, &event) < 0) {
      _log.error("WORKER (" + Utils::to_string(_thread) +
                 "): Failed \"epoll_ctl\"");
      continue;
    }
  }
}

void Worker::_handleIncomingConnection(struct epoll_event event) {
  ConnectionHandler* handler = static_cast<ConnectionHandler*>(event.data.ptr);
  _log.info("WORKER (" + Utils::to_string(_thread) +
            "): Handling incoming connection from fd: " + Utils::to_string(event.data.fd));
  handler->processConnection();
  _log.info("WORKER (" + Utils::to_string(_thread) +
            "): Connection processed");
}

void* Worker::_workerRoutine(void* ref) {
  Worker* worker = static_cast<Worker*>(ref);
  worker->_threadId = gettid();
  worker->_log.info("WORKER (" + Utils::to_string(worker->_threadId) +
                    "): Started");
  worker->_runEventLoop();
  worker->_log.info("WORKER (" + Utils::to_string(worker->_threadId) +
                    "): Finished");
  worker->_shouldStop = true;
  worker->_server.workerFinished();
  return NULL;
}
