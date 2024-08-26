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
      _listenSockets(listenSockets) {
}

Worker::~Worker() {
}

void Worker::processEvent(struct epoll_event &event) {
    EventData* eventData = static_cast<EventData*>(event.data.ptr);
    if (eventData && eventData->isListening && (event.events & EPOLLIN)) {
      _acceptNewConnection(eventData->fd);
    } else if (eventData && event.events) {
      _launchEventProcessing(eventData, event);
    }
    else {
      _log.warning("WORKER: Unable to handle event with no data");
      epoll_ctl(_epollSocket, EPOLL_CTL_DEL, event.data.fd, NULL);
      close(event.data.fd);
    }
}

void Worker::_launchEventProcessing(EventData* eventData, struct epoll_event& event) {
  if (!eventData->handler) {
    _log.warning("WORKER: Handler is NULL for event fd: " + Utils::to_string(event.data.fd) +
                 " -> " + Utils::to_string(eventData->fd));
    return;
  }
  try {
    _log.info("WORKER: Launching event processing for fd: " + Utils::to_string(eventData->fd));
    eventData->handler->processConnection(event);
  } catch (std::exception& e) {
    _log.error("WORKER: Exception: " + std::string(e.what()));
  }
}

void Worker::_acceptNewConnection(int fd) {
  struct sockaddr_storage address;
  socklen_t               addrlen = sizeof(address);
  int                     new_socket;
  struct epoll_event      event;

  memset(&address, 0, sizeof(address));
  do {
    new_socket = accept(fd, (struct sockaddr*)&address,
                        &addrlen);  // creer un nouveau sockect d'echange d'event
    if (new_socket < 0) {
      break;
    }
    if (Utils::set_non_blocking(new_socket) == -1) {
      _log.error("WORKER: Failed \"set_non_blocking\" on new socket " +
                 Utils::to_string(new_socket));
      close(new_socket);
      return;
    }
    _log.info("WORKER: Accepted new connection: " + Utils::to_string(new_socket));
    ListenConfig                listenConfig = _listenSockets[fd];
    std::vector<VirtualServer*> virtualServers =
        _setupAssociatedVirtualServers(listenConfig);
    ConnectionHandler* handler = new ConnectionHandler(
        new_socket, _epollSocket, virtualServers, listenConfig);
    EventData* eventData = new EventData(new_socket, handler);
    event.data.ptr = eventData;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, new_socket, &event) < 0) {
      _log.error("WORKER: Failed \"epoll_ctl\" on new socket " + Utils::to_string(new_socket));
      eventData->opened = false;
    }
  } while (new_socket > 0);
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
