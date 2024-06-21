/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:02 by agaley            #+#    #+#             */
/*   Updated: 2024/05/07 09:16:32 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Common.hpp"
#include "ConfigManager.hpp"
#include "Utils.hpp"
#include <climits>

Server* Server::_instance = NULL;
int Server::_callCount = 1;

Server::Server()
    : _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      _activeWorkers(0) {
  _log.info("====================SERVER: Setup server " + Utils::to_string(_callCount));
  _setupEpoll();
  _setupServerSockets();
  _setupWorkers();
  pthread_mutex_init(&_mutex, NULL);
  pthread_cond_init(&_cond, NULL);
  _callCount++;
}

Server::~Server() {
  for (size_t i = 0; i < _workers.size(); i++) {
    _workers[i]->stop();
    delete _workers[i];
  }
  _workers.clear();
  pthread_cond_destroy(&_cond);
  pthread_mutex_destroy(&_mutex);
  Server::_instance = NULL;
}

Server& Server::getInstance() {
  if (_instance == NULL) {
    _instance = new Server();
  }
  return *_instance;
}

void Server::workerFinished() {
  pthread_mutex_lock(&_mutex);
  _activeWorkers--;
  if (_activeWorkers == 0) {
    pthread_cond_signal(&_cond);
  }
  pthread_mutex_unlock(&_mutex);
}

void Server::start() {
  for (size_t i = 0; i < _workers.size(); i++) {
    _workers[i]->start();
    pthread_mutex_lock(&_mutex);
    _activeWorkers++;
    pthread_mutex_unlock(&_mutex);
  }
  // pthread_mutex_lock(&_mutex);
  // while (_activeWorkers > 0) {
  //   pthread_cond_wait(&_cond, &_mutex);
  // }
  // pthread_mutex_unlock(&_mutex);
  struct epoll_event events[MAX_EVENTS];
  while (true) {
    int nfds = epoll_wait(_epollSocket, events, MAX_EVENTS, -1);
    if (nfds < 0) {
      _log.error("SERVER: Failed to wait for events");
      continue;
    }
    for (int i = 0; i < nfds; i++) {
    std::string eventDetails = "=========>>>SERVER: Dispatching event for fd " + Utils::to_string(events[i].data.fd) + ": ";

    if (events[i].events & EPOLLIN) {
        eventDetails += "EPOLLIN ";
    }
    if (events[i].events & EPOLLOUT) {
        eventDetails += "EPOLLOUT ";
    }
    if (events[i].events & EPOLLERR) {
        eventDetails += "EPOLLERR ";
    }
    if (events[i].events & EPOLLHUP) {
        eventDetails += "EPOLLHUP ";
    }
    if (events[i].events & EPOLLRDHUP) {
        eventDetails += "EPOLLRDHUP ";
    }
    if (events[i].events & EPOLLET) {
        eventDetails += "EPOLLET ";
    }
    if (events[i].events & EPOLLONESHOT) {
        eventDetails += "EPOLLONESHOT ";
    }

    _log.info(eventDetails);
    _dispatchEvent(events[i]);
}
  }
}

void Server::_dispatchEvent(struct epoll_event event) {
    Worker* bestChoice = NULL;
    int lowestLoad = INT_MAX;

    // Parcourir tous les workers pour trouver le meilleur choix
    std::vector<Worker*>::iterator it;
    for (it = _workers.begin(); it != _workers.end(); ++it) {
        int workerLoad = (*it)->getLoad();

        // Si un worker avec une charge de 0 est trouvé, sélectionnez-le immédiatement
        if (workerLoad == 0) {
            bestChoice = *it;
            break; // Arrêtez la recherche car vous avez trouvé le worker idéal
        }

        // Sinon, continuez à chercher le worker avec la charge minimale
        if (workerLoad < lowestLoad) {
            lowestLoad = workerLoad;
            bestChoice = *it;
        }
    }

    // Si un worker a été sélectionné, lui assigner l'événement
    if (bestChoice != NULL) {
        bestChoice->pushEvent(event);
        _log.info("SERVER: Dispatched event to worker with load " + Utils::to_string(bestChoice->getLoad()) + " for fd " + Utils::to_string(event.data.fd));
    } else {
        _log.error("SERVER: No available worker to dispatch event for fd " + Utils::to_string(event.data.fd));
    }
}

void Server::stop(int signum) {
  if (signum == SIGINT || signum == SIGTERM || signum == SIGKILL) {
    Server::_instance->_log.info("Server stopped from signal " + Utils::to_string(signum));
    delete Server::_instance;
    Server::_instance = NULL;
    throw std::runtime_error("Server stopped");
  }
}

void Server::_setupWorkers() {
  for (int i = 0; i < _config.worker_processes; i++) {
    _workers.push_back(new Worker(_epollSocket, _listenSockets));
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

  _log.info("SERVER: Setup server sockets");
  for (std::set<ListenConfig>::const_iterator it = uniqueConfigs.begin();
       it != uniqueConfigs.end(); ++it) {
    const ListenConfig& listenConfig = *it;
    int                 sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0) {
      _log.error("(" + listenConfig.address + ":" +
      Utils::to_string(listenConfig.port) +
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
      if (inet_pton(AF_INET6, listenConfig.address.c_str(),
                    &address.sin6_addr) <= 0) {
        _log.error("(" + listenConfig.address + ":" +
                   Utils::to_string(listenConfig.port) + ") Invalid address");
        close(sock);
        continue;
      }
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)) < 0) {
      _log.error("(" + listenConfig.address + ":" +
                 Utils::to_string(listenConfig.port) +
                 ") Failed to set socket options");
      close(sock);
      continue;
    }

    if (listenConfig.ipv6only) {
      int ipv6only = 1;
      if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6only,
                     sizeof(ipv6only)) < 0) {
        _log.error("(" + listenConfig.address + ":" +
                   Utils::to_string(listenConfig.port) +
                   ") Failed to set IPV6_V6ONLY");
        close(sock);
        continue;
      }
    }

    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &listenConfig.rcvbuf,
                   sizeof(listenConfig.rcvbuf)) < 0) {
      _log.error("(" + listenConfig.address + ":" +
                 Utils::to_string(listenConfig.port) +
                 ") Failed to set SO_REUSEPORT");
      close(sock);
      continue;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &listenConfig.sndbuf,
                   sizeof(listenConfig.sndbuf)) < 0) {
      _log.error("(" + listenConfig.address + ":" +
                 Utils::to_string(listenConfig.port) +
                 ") Failed to set SO_SNDBUF");
      close(sock);
      continue;
    }

    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
      _log.error("(" + listenConfig.address + ":" +
                 Utils::to_string(listenConfig.port) +
                 ") Failed to bind socket");
      close(sock);
      continue;
    }

    if (listen(sock, listenConfig.backlog) < 0) {
      _log.error("(" + listenConfig.address + ":" +
                 Utils::to_string(listenConfig.port) +
                 ") Failed to listen on socket");
      close(sock);
      continue;
    }

    if (set_non_blocking(sock) < 0) {
      _log.error("(" + listenConfig.address + ":" +
                 Utils::to_string(listenConfig.port) +
                 ") Failed to set socket to non-blocking");
      close(sock);
      continue;
    }

    struct epoll_event event;
    event.data.fd = sock;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, sock, &event) == -1) {
      close(sock);
      _log.error(std::string("SERVER (assign conn): Failed \"epoll_ctl\": ") +
                 strerror(errno) + " (" + Utils::to_string(sock) + ")");
      continue;
    }
    _log.info("SERVER (assign conn): Add socket to epoll : " + Utils::to_string(sock));
    _listenSockets[sock] = listenConfig;
  }
}
