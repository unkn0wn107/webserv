/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 12:06:51 by mchenava          #+#    #+#             */
/*   Updated: 2024/07/23 19:35:41 by agaley           ###   ########lyon.fr   */
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
#include <map>
#include <queue>

#include "Common.hpp"
#include "ConnectionHandler.hpp"
#include "EventData.hpp"
#include "EventQueue.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include "VirtualServer.hpp"

class Server;

class Worker {
 public:
  Worker(Server&                      server,
         int                          epollSocket,
         std::map<int, ListenConfig>& listenSockets,
         EventQueue&                  events);
  ~Worker();

  void                        processEvent(struct epoll_event &event);

 private:
  void                        _acceptNewConnection(int fd);
  std::vector<VirtualServer*> _setupAssociatedVirtualServers(
      const ListenConfig& listenConfig);
  void _launchEventProcessing(EventData* eventData, struct epoll_event& event);


  Server&                      _server;
  EventQueue&                  _events;
  std::map<int, EventData*>    _eventsData;
  const Config&                _config;
  Logger&                      _log;
  int                          _epollSocket;
  std::map<int, ListenConfig>& _listenSockets;

  Worker(const Worker&);
};

#endif
