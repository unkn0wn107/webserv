/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 12:06:51 by mchenava          #+#    #+#             */
/*   Updated: 2024/06/24 14:40:55 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WORKER_HPP
#define WORKER_HPP

#include <pthread.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <queue>

#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include "Common.hpp"
#include "VirtualServer.hpp"
#include "Server.hpp"


#define MAX_EVENTS 10
#define WORKER_TIME_TO_STOP 2

class ConnectionHandler;
class Server;

class Worker {
 private:
  Server&                                     _server;
  pthread_t                                   _thread;
  int                                         _threadId;
  Config&                                     _config;
  Logger&                                     _log;
  std::map<int, ConnectionHandler*>           _handlers;
  std::map<int, std::vector<VirtualServer*> > _virtualServers;
  int                                         _epollSocket;
  std::map<int, ListenConfig>                 _listenSockets;
  // int                                         _maxConnections;
  int                                         _load;
  bool                                        _shouldStop;
  // pthread_mutex_t                              _queueMutex;

  static void* _workerRoutine(void* ref);

  void                        _setupEpoll();
  std::vector<VirtualServer*> _setupAssociateVirtualServer(
      const ListenConfig& listenConfig);
  void _acceptNewConnection(int fd);
  void _runEventLoop();
  void _handleIncomingConnection(struct epoll_event event);

 public:
  Worker(Server& server, int epollSocket, std::map<int, ListenConfig>& listenSockets);
  ~Worker();
  void stop();
  void start();
  // void pushEvent(struct epoll_event event);
  int getLoad();
};

#endif
