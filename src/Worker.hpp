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
#include "EventQueue.hpp"
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
         EventQueue&                  events);
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

  // class Mutex {
  //  public:
  //   Mutex();
  //   ~Mutex();

  //   void lock();
  //   void unlock();

  //  private:
  //   pthread_mutex_t _mutex;
  // };

  static void*                _workerRoutine(void* arg);
  void                        _runEventLoop();
  void                        _acceptNewConnection(int fd);
  void                        _handleIncomingConnection(epoll_event& event);
  std::vector<VirtualServer*> _setupAssociatedVirtualServers(
      const ListenConfig& listenConfig);
  void _launchEventProcessing(EventData* eventData, struct epoll_event& event);
  void _pushBackToQueue(EventData* eventData, const struct epoll_event& event);

  void _cleanUpForceResponse();
  void _cleanUpSendings();
  void _cleanUpAll();

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
  // Mutex                        _mutex;
  pid_t _threadId;

  Worker(const Worker&);
  Worker& operator=(const Worker&);
};

#endif
