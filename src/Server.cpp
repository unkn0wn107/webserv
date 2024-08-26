/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:02 by agaley            #+#    #+#             */
/*   Updated: 2024/07/24 19:25:25 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include <climits>

#include "CacheHandler.hpp"
#include "Common.hpp"
#include "ConfigManager.hpp"
#include "EventQueue.hpp"
#include "Server.hpp"
#include "Utils.hpp"

Server* Server::_instance = NULL;

Server::Server()
    : _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      _listenSockets(),
      _listenEventData(),
      _events(),
      _running(false),
      _requestTimes() {
  _setupEpoll();
  _setupServerSockets();
  _worker = new Worker(*this, _epollSocket, _listenSockets, _events);
  _instance = this;
}

Server::~Server() {
  delete _worker;
  close(_epollSocket);
  for (std::map<int, ListenConfig>::iterator it = _listenSockets.begin();
       it != _listenSockets.end(); ++it)
    close(it->first);
  for (std::set<EventData*>::iterator it = _listenEventData.begin();
       it != _listenEventData.end(); ++it)
    delete *it;
  _listenSockets.clear();
  _clearRequests();
  _listenSockets.clear();
  CacheHandler::deleteInstance();
  ConfigManager::deleteInstance();
  Server::_instance = NULL;
}

Server& Server::getInstance() {
  if (_instance == NULL) {
    Logger::getInstance().info("SERVER: Creating server instance");
    _instance = new Server();
  }
  return *_instance;
}

void Server::start() {
  _running = true;
  struct epoll_event events[MAX_EVENTS];
  while (_running) {
    _checkRequestLife();
    int nfds = epoll_wait(_epollSocket, events, MAX_EVENTS, 10);
    if (nfds > 0) {
      _log.info("SERVER: epoll_wait: " + Utils::to_string(nfds) + " events");
    }
    for (int i = 0; i < nfds && _running; i++)
    {
      EventData* eventData = static_cast<EventData*>(events[i].data.ptr);
      if (!eventData->isListening && eventData->recordTime) {
        eventData->startTime = time(NULL);
        _requestTimes.push_back(eventData);
        eventData->recordTime = false;
      }
      _worker->processEvent(events[i]);
    }
  }
}

void Server::_checkRequestLife() {
  for (std::list<EventData*>::iterator it = _requestTimes.begin(); it != _requestTimes.end(); ) {
    EventData* data = *it;
    if ((time(NULL) - data->startTime) > TIMEOUT) {
      _log.warning("SERVER: Request timed out (" + Utils::to_string(data->fd) + ")");
      HTTPResponse::sendResponse(HTTPResponse::GATEWAY_TIMEOUT, data->fd);
      epoll_ctl(_epollSocket, EPOLL_CTL_DEL, data->fd, NULL);
      close(data->fd);
      delete data->handler;
      delete data;
      it = _requestTimes.erase(it);
    }
    else if (!data->opened) {
      epoll_ctl(_epollSocket, EPOLL_CTL_DEL, data->fd, NULL);
      close(data->fd);
      delete data->handler;
      delete data;
      it = _requestTimes.erase(it);
    } else {
      ++it;
    }
  }
}

void Server::_clearRequests() {
  for (std::list<EventData*>::iterator it = _requestTimes.begin(); it != _requestTimes.end(); ) {
      epoll_ctl(_epollSocket, EPOLL_CTL_DEL, (*it)->fd, NULL);
      close((*it)->fd);
      delete (*it)->handler;
      delete *it;
      ++it;
  }
  _requestTimes.clear();
}

void Server::stop(int signum) {
  if (signum == SIGINT || signum == SIGTERM) {
    _running = false;
  }
}

void Server::_setupEpoll() {
  _epollSocket = epoll_create1(0);
  if (_epollSocket == -1) {
    _log.error("Failed to create epoll socket");
    throw std::runtime_error("Failed to create epoll socket");
  }
}

void Server::_setupServerSockets() {
  const std::set<ListenConfig>& uniqueConfigs = _config.unique_listen_configs;

  for (std::set<ListenConfig>::const_iterator it = uniqueConfigs.begin();
       it != uniqueConfigs.end(); ++it) {
    const ListenConfig& listenConfig = *it;
    int                 sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0) {
      _log.error("(" + listenConfig.address + ":" + Utils::to_string(listenConfig.port) +
                 ") Failed to create socket");
      continue;
    }

    struct sockaddr_in6 address;
    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(listenConfig.port);

    if (listenConfig.address.empty() || listenConfig.address == "*") {
      address.sin6_addr = in6addr_any;
    } else {
      if (inet_pton(AF_INET6, listenConfig.address.c_str(), &address.sin6_addr) <= 0) {
        _log.error("(" + listenConfig.address + ":" +
                   Utils::to_string(listenConfig.port) + ") Invalid address");
        close(sock);
        continue;
      }
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) <
        0) {
      _log.error("(" + listenConfig.address + ":" + Utils::to_string(listenConfig.port) +
                 ") Failed to set socket options");
      close(sock);
      continue;
    }

    if (listenConfig.ipv6only) {
      int ipv6only = 1;
      if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6only, sizeof(ipv6only)) < 0) {
        _log.error("(" + listenConfig.address + ":" +
                   Utils::to_string(listenConfig.port) + ") Failed to set IPV6_V6ONLY");
        close(sock);
        continue;
      }
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &listenConfig.rcvbuf,
                   sizeof(listenConfig.rcvbuf)) < 0) {
      _log.error("(" + listenConfig.address + ":" + Utils::to_string(listenConfig.port) +
                 ") Failed to set SO_REUSEPORT");
      close(sock);
      continue;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &listenConfig.sndbuf,
                   sizeof(listenConfig.sndbuf)) < 0) {
      _log.error("(" + listenConfig.address + ":" + Utils::to_string(listenConfig.port) +
                 ") Failed to set SO_SNDBUF");
      close(sock);
      continue;
    }

    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
      _log.error("(" + listenConfig.address + ":" + Utils::to_string(listenConfig.port) +
                 ") Failed to bind socket");
      close(sock);
      continue;
    }

    if (listen(sock, listenConfig.backlog) < 0) {
      _log.error("(" + listenConfig.address + ":" + Utils::to_string(listenConfig.port) +
                 ") Failed to listen on socket");
      close(sock);
      continue;
    }

    if (Utils::set_non_blocking(sock) < 0) {
      _log.error("(" + listenConfig.address + ":" + Utils::to_string(listenConfig.port) +
                 ") Failed to set socket to non-blocking");
      close(sock);
      continue;
    }

    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    EventData* eventData = new EventData(sock, NULL, true);
    event.data.ptr = eventData;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, sock, &event) == -1) {
      close(sock);
      delete eventData;
      _log.error(std::string("SERVER (assign conn): Failed \"epoll_ctl\": ") +
                 strerror(errno) + " (" + Utils::to_string(sock) + ")");
      continue;
    }
    _listenSockets[sock] = listenConfig;
    _listenEventData.insert(eventData);
  }
}
