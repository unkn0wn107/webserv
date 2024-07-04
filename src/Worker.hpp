/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 12:06:51 by mchenava          #+#    #+#             */
/*   Updated: 2024/07/04 03:23:04 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef WORKER_HPP
#define WORKER_HPP

#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <queue>
#include <map>

#include "Common.hpp"
#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include "VirtualServer.hpp"

class Server;

#define MAX_EVENTS 1024
#define WORKER_TIME_TO_STOP 2

class Worker {
 public:
  Worker(Server&                      server,
         int                          epollSocket,
         std::map<int, ListenConfig>& listenSockets);
  ~Worker();

  void processEvent(struct epoll_event& event);
  int  getLoad() const;

 private:
  void         _acceptNewConnection(int fd);
  void         _handleIncomingConnection(epoll_event& event);
  std::vector<VirtualServer*> _setupAssociatedVirtualServers(
      const ListenConfig& listenConfig);

  Server&                      _server;
  const Config&                _config;
  Logger&                      _log;
  int                          _epollSocket;
  std::map<int, ListenConfig>& _listenSockets;
  int                          _load;

  Worker(const Worker&);
  Worker& operator=(const Worker&);
};

#endif
