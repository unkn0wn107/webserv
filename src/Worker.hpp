/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 12:06:51 by mchenava          #+#    #+#             */
/*   Updated: 2024/06/11 15:53:12 by  mchenava        ###   ########.fr       */
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

#include "ConnectionHandler.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include "VirtualServer.hpp"

#define MAX_EVENTS 10
#define WORKER_TIME_TO_STOP 2

class ConnectionHandler;

class Worker {
 private:
  pthread_t                                   _thread;
  Config&                                     _config;
  Logger&                                     _log;
  std::map<int, ConnectionHandler*>           _handlers;
  std::map<int, ListenConfig>                 _listenConfigs;
  std::map<int, std::vector<VirtualServer*> > _virtualServers;
  int                                         _epollSocket;
  std::vector<int>                            _listenSockets;
  int                                         _maxConnections;
  int                                         _currentConnections;
  bool                                        _shouldStop;

  static void* _workerRoutine(void* ref);

  void                        _setupEpoll();
  std::vector<VirtualServer*> _setupAssociateVirtualServer(
      const ListenConfig& listenConfig);
  void _acceptNewConnection(int fd);
  void _runEventLoop();
  void _handleIncomingConnection(struct epoll_event& event);

 public:
  Worker();
  ~Worker();
  void assignConnection(int clientSocket, const ListenConfig& listenConfig);
  void stop();
  void start();
};

#endif
