/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/06/27 18:07:06 by agaley           ###   ########lyon.fr   */
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
#include <set>

#include "Common.hpp"
#include "Config.hpp"
#include "EventQueue.hpp"
#include "Logger.hpp"
#include "Worker.hpp"

#define SHUTDOWN_DELAY 200000

class Worker;

class Server {
 private:
  static int                                  _callCount;
  static Server*                              _instance;
  Config&                                     _config;
  Logger&                                     _log;
  std::vector<Worker*>                        _workers;
  std::map<int, ListenConfig>                 _listenSockets;
  pthread_mutex_t                             _mutex;
  int                                         _epollSocket;
  int                                         _activeWorkers;
  pthread_mutex_t                             _eventsMutex;
  bool                                        _running;
  EventQueue                                  _events;
  std::map<int, std::vector<VirtualServer*> > _virtualServers;

  void _setupServerSockets();
  void _setupWorkers();
  void _setupEpoll();

 public:
  Server();
  static Server& getInstance();
  void           workerFinished();
  ~Server();

  void start();
  void stop(int signum);
};

#endif
