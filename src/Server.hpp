/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/07/23 01:51:20 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <memory>
#include <set>
#include <list>

#include "Common.hpp"
#include "Config.hpp"
#include "EventData.hpp"
#include "EventQueue.hpp"
#include "Logger.hpp"
#include "Worker.hpp"

class Worker;

class Server {
 private:
  static int                                  _callCount;
  static Server*                              _instance;
  Config&                                     _config;
  Logger&                                     _log;
  std::vector<Worker*>                        _workers;
  std::map<int, ListenConfig>                 _listenSockets;
  std::set<EventData*>                        _listenEventData;
  pthread_mutex_t                             _mutex;
  pthread_mutex_t                             _requestTimesMutex;
  int                                         _epollSocket;
  int                                         _activeWorkers;
  EventQueue                                  _events;
  bool                                        _running;
  std::map<int, std::vector<VirtualServer*> > _virtualServers;
  std::list<EventData*>                       _requestTimes;

  void _setupServerSockets();
  void _setupWorkers();
  void _setupEpoll();
  void _checkRequestLife();

 public:
  Server();
  static Server& getInstance();
  void           workerFinished();
  ~Server();

  void start();
  void stop(int signum);
};

#endif
