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
               std::map<int, ListenConfig>& listenSockets,
               EventQueue& events)
    : _server(server),
      _events(events),
      _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      _epollSocket(epollSocket),
      _listenSockets(listenSockets),
      _load(0),
      _shouldStop(false) {
  _log.info("Worker constructor called");
}

Worker::~Worker() {}

Worker::Thread::Thread() : _thread(0) {}

Worker::Thread::~Thread() {
  if (_thread)
    pthread_detach(_thread);
}

bool Worker::Thread::create(void* (*start_routine)(void*), void* arg) {
  return pthread_create(&_thread, NULL, start_routine, arg) == 0;
}

void Worker::start() {
  if (!_thread.create(_workerRoutine, this)) {
    _log.error("WORKER : Failed to create thread");
  }
}

void Worker::stop() {
  _shouldStop = true;
}

pid_t Worker::getThreadId() const {
  return _threadId;
}

int Worker::getLoad() const {
  return _load;
}

void* Worker::_workerRoutine(void* arg) {
  Worker* worker = static_cast<Worker*>(arg);
  worker->_threadId = gettid();
  worker->_log.info("WORKER (" + Utils::to_string(worker->_threadId) +
                    "): Started");
  worker->_runEventLoop();
  worker->_log.info("WORKER (" + Utils::to_string(worker->_threadId) +
                    "): Finished");
  worker->_server.workerFinished();
  return NULL;
}

void Worker::_runEventLoop() {
  while (!_shouldStop) {
    struct epoll_event event;

    if (!_events.try_pop(event)) {
      usleep(1000);
      continue;
    }

    _log.info("WORKER (" + Utils::to_string(_threadId) +
              "): Event: " + Utils::to_string(event.events));

    if (_listenSockets.find(event.data.fd) != _listenSockets.end() && (event.events & EPOLLIN)) {
        _acceptNewConnection(event.data.fd);
    } else {
        ConnectionHandler* handler = static_cast<ConnectionHandler*>(event.data.ptr);
        _log.info("WORKER (" + Utils::to_string(_threadId) +
                  "): Handler: " + (handler ? "Valid" : "Invalid"));
        if (handler && _handlers.find(handler->getSocket()) != _handlers.end() && event.events) {
            if (handler->processConnection() == 1) {
              _handlers.erase(handler->getSocket());
              _log.warning("WORKER (" + Utils::to_string(_threadId) +
                        "): Handler deleted with con status " + Utils::to_string(handler->getConnectionStatus()));
              delete handler;
              continue;
            }
        } else {
          if (handler->getSocket() > 0) {
            _log.info("WORKER (" + Utils::to_string(_threadId) +
                      "): Event pushed back to Queue " + Utils::to_string(handler->getSocket()));
            _events.push(event);
            usleep(500);
            continue;
          }
        }
    }

    std::clock_t _start = std::clock();
    std::clock_t end = std::clock();
    double duration = static_cast<double>(end - _start) / CLOCKS_PER_SEC;
    if (duration > 0.005) {
      _log.warning("WORKER (" + Utils::to_string(_threadId) +
                   "): Event loop time: " + Utils::to_string(duration) +
                   " seconds");
    }
  }

  _log.info("WORKER (" + Utils::to_string(_threadId) +
            "): Cleaning up handlers");
  for (std::map<int, ConnectionHandler*>::iterator it = _handlers.begin(); it != _handlers.end(); ++it) {
    it->second->closeClientSocket();
    delete it->second;
  }
  _handlers.clear(); // Ensure all handler pointers are removed after deletion
  _log.info("WORKER (" + Utils::to_string(_threadId) + "): End of event loop");
}

void Worker::_acceptNewConnection(int fd) {
  struct sockaddr_storage address;
  socklen_t               addrlen = sizeof(address);
  int                     new_socket;
  struct epoll_event      event;

  memset(&address, 0, sizeof(address));
  memset(&event, 0, sizeof(event));
  while (!_shouldStop) {
    new_socket = accept(fd, (struct sockaddr*)&address, &addrlen); //creer un nouveau sockect d'echange d'event
    if (new_socket <= 0) {
      break;
    }
    if (set_non_blocking(new_socket) == -1) {
      _log.error("WORKER (" + Utils::to_string(_threadId) +
                 "): Failed \"set_non_blocking\"");
      continue;
    }
    _log.info("WORKER (" + Utils::to_string(_threadId) +
              "): Accepted new connection: " + Utils::to_string(new_socket));
    ListenConfig                listenConfig = _listenSockets[fd];
    std::vector<VirtualServer*> virtualServers =
        _setupAssociatedVirtualServers(listenConfig);
    ConnectionHandler* handler = new ConnectionHandler(
        new_socket, _epollSocket, virtualServers, listenConfig);
    _handlers[new_socket] = handler;
    event.data.ptr = handler;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, new_socket, &event) < 0) {
      _log.error("WORKER (" + Utils::to_string(_threadId) +
                 "): Failed \"epoll_ctl\"");
      close(new_socket);
      continue;
    }
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
