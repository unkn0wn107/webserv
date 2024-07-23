/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 12:06:51 by mchenava          #+#    #+#             */
/*   Updated: 2024/07/23 02:08:54 by  mchenava        ###   ########.fr       */
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
         EventQueue&                  events,
         pthread_mutex_t&             requestTimesMutex);
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
  void _launchEventProcessing(EventData* eventData, struct epoll_event& event);

  Server&                      _server;
  EventQueue&                  _events;
  Thread                       _thread;
  std::map<int, EventData*>    _eventsData;
  const Config&                _config;
  Logger&                      _log;
  int                          _epollSocket;
  std::map<int, ListenConfig>& _listenSockets;
  int                          _load;
  bool                         _shouldStop;
  pid_t                        _threadId;
  pthread_mutex_t&              _requestTimesMutex;

  Worker(const Worker&);
};

#endif
