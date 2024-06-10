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

Server* Server::_instance = NULL;

Server::Server()
    : _config(ConfigManager::getInstance().getConfig()),
      _log(Logger::getInstance()),
      _workerIndex(0) {
  Server::_instance = this;

  struct sigaction sigHandler;
  sigHandler.sa_handler = _signalHandler;
  sigemptyset(&sigHandler.sa_mask);
  sigHandler.sa_flags = 0;

  sigaction(SIGINT, &sigHandler, NULL);
  sigaction(SIGTERM, &sigHandler, NULL);

  pthread_mutex_init(&_epollMutex, NULL);
  _setupWorkers();
  _setupServerSockets();
}

Server::~Server() {
  for (size_t i = 0; i < _workers.size(); i++) {
    delete _workers[i];
  }
  _workers.clear();
  pthread_mutex_destroy(&_epollMutex);
  Server::_instance = NULL;
}

void Server::_signalHandler(int signum) {
  if (signum == SIGINT || signum == SIGTERM) {
    delete Server::_instance;
    // Server::_instance = NULL;
    throw std::runtime_error("Server stopped");
  }
}

void Server::_setupWorkers() {
  for (int i = 0; i < _config.worker_processes; i++) {
    _workers.push_back(new Worker());
  }
}

void Server::_setupServerSockets() {
  const std::set<ListenConfig>& uniqueConfigs = _config.unique_listen_configs;

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
    _listenSockets[listenConfig] = sock;
    _workers[_workerIndex]->assignConnection(sock, listenConfig);
    _workerIndex = (_workerIndex + 1) % _config.worker_processes;
  }
}
