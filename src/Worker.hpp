/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 12:06:51 by mchenava          #+#    #+#             */
/*   Updated: 2024/07/05 01:40:26 by agaley           ###   ########lyon.fr   */
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
#include <ctime>
#include <iostream>
#include <map>
#include <queue>

#include "Common.hpp"
#include "ConnectionHandler.hpp"
#include "EventData.hpp"
#include "SPMCQueue.hpp"
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
         std::map<int, ListenConfig>& listenSockets,
         SPMCQueue<struct epoll_event>& events);
  ~Worker();

  void  start();
  void  stop();
  pid_t getThreadId() const;
  int   getLoad() const;

 private:
  class Thread {
   public:
    Thread();
    ~Thread();
    bool create(void* (*start_routine)(void*), void* arg);

   private:
    pthread_t _thread;
  };

  static void*                _workerRoutine(void* arg);
  void                        _runEventLoop();
  void                        _acceptNewConnection(int fd);
  std::vector<VirtualServer*> _setupAssociatedVirtualServers(
      const ListenConfig& listenConfig);
  void                        _launchEventProcessing(
      EventData* eventData, struct epoll_event& event);

  Server&                             _server;
  SPMCQueue<struct epoll_event>&      _events;
  Thread                              _thread;
  std::map<int, EventData*>           _eventsData;
  const Config&                       _config;
  Logger&                             _log;
  int                                 _epollSocket;
  std::map<int, ListenConfig>&        _listenSockets;
  int                                 _load;
  bool                                _shouldStop;
  pid_t                               _threadId;

  Worker(const Worker&);
  Worker& operator=(const Worker&);
};

#endif
