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
#include "EventData.hpp"

Worker::Worker(Server&                      server,
               int                          epollSocket,
               std::map<int, ListenConfig>& listenSockets,
               EventQueue&                  events)
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

    EventData* eventData = static_cast<EventData*>(event.data.ptr);
    if (eventData && eventData->isListening && (event.events & EPOLLIN)) {
      _acceptNewConnection(eventData->fd);
    }
    else if (eventData && event.events)
      _launchEventProcessing(eventData, event);
    else {
      _log.warning("WORKER (" + Utils::to_string(_threadId) +
                   "): Unable to handle event with no data");
      epoll_ctl(_epollSocket, EPOLL_CTL_DEL, event.data.fd, NULL);
      close(event.data.fd);
      // delete eventData->handler;
      // delete eventData;
    }
  }
  // _cleanUpForceResponse();
  // _cleanUpSendings();
  // _cleanUpAll();
}

void Worker::_launchEventProcessing(EventData*          eventData,
                                    struct epoll_event& event) {
  int handlerStatus = 0;
  if (!eventData->handler) {
    _log.warning("WORKER (" + Utils::to_string(_threadId) +
                 "): Handler is NULL for event fd: " + Utils::to_string(event.data.fd) +
                 " -> " + Utils::to_string(eventData->fd));
    return;
  }
  try {
    handlerStatus = eventData->handler->processConnection(event);
  } catch (std::exception& e) {
    _log.error("WORKER (" + Utils::to_string(_threadId) +
               "): Exception: " + e.what());
  }
  if (handlerStatus == 1) {  // Done
    _log.warning("WORKER (" + Utils::to_string(_threadId) +
                 "): Handler deleted with con status " +
                 eventData->handler->getStatusString());
    delete eventData->handler;
    delete eventData;
  }
}

void Worker::_acceptNewConnection(int fd) {
  struct sockaddr_storage address;
  socklen_t               addrlen = sizeof(address);
  int                     new_socket;
  struct epoll_event      event;

  memset(&address, 0, sizeof(address));
  while (!_shouldStop) {
    new_socket =
        accept(fd, (struct sockaddr*)&address,
               &addrlen);  // creer un nouveau sockect d'echange d'event
    if (new_socket <= 0) {
      _log.warning("WORKER (" + Utils::to_string(_threadId) +
                   "): Failed \"accept\" on listening socket " +
                   Utils::to_string(fd));
      return;
    }
    if (set_non_blocking(new_socket) == -1) {
      _log.error("WORKER (" + Utils::to_string(_threadId) +
                 "): Failed \"set_non_blocking\" on new socket " +
                 Utils::to_string(new_socket));
      close(new_socket);
      return;
    }
    _log.info("WORKER (" + Utils::to_string(_threadId) +
              "): Accepted new connection: " + Utils::to_string(new_socket));
    ListenConfig                listenConfig = _listenSockets[fd];
    std::vector<VirtualServer*> virtualServers =
        _setupAssociatedVirtualServers(listenConfig);
    ConnectionHandler* handler = new ConnectionHandler(
        new_socket, _epollSocket, virtualServers, listenConfig, _events);
    EventData* eventData = new EventData(new_socket, handler, _threadId);
    event.data.ptr = eventData;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, new_socket, &event) < 0) {
      _log.error("WORKER (" + Utils::to_string(_threadId) +
                 "): Failed \"epoll_ctl\" on new socket " +
                 Utils::to_string(new_socket));
      close(new_socket);
      delete handler;
      delete eventData;
    }
  }
}

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

// void Worker::_cleanUpForceResponse() {
//   bool hasOtherStatus = true;
//   while (hasOtherStatus) {
//     hasOtherStatus = false;
//     for (std::map<int, EventData*>::iterator it = _eventsData.begin();
//          it != _eventsData.end(); ++it) {
//       if (!it->second->handler) {
//         _eventsData.erase(it);
//         continue;
//       }
//       ConnectionStatus status = it->second->handler->getConnectionStatus();
//       if (status != SENDING && status != CLOSED) {
//         it->second->handler->setInternalServerError();
//         hasOtherStatus = true;
//       }
//     }
//   }
// }

// void Worker::_cleanUpSendings() {
//   bool hasSending = true;
//   while (hasSending) {
//     hasSending = false;
//     usleep(500);
//     for (std::map<int, EventData*>::iterator it = _eventsData.begin();
//          it != _eventsData.end(); ++it) {
//       if (!it->second->handler) {
//         _eventsData.erase(it);
//         continue;
//       }
//       if (it->second->handler->getConnectionStatus() == SENDING) {
//         it->second->handler->forceSendResponse();
//         hasSending = true;
//       }
//     }
//   }
// }

void Worker::_cleanUpAll() {
  for (std::map<int, EventData*>::iterator it = _eventsData.begin();
       it != _eventsData.end(); ++it) {
    if (!it->second->handler)
      continue;
    it->second->handler->closeClientSocket();
    delete it->second->handler;
    delete it->second;
    _eventsData.erase(it);
  }
}
