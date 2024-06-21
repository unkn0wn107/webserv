/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/06/21 12:02:17 by  mchenava        ###   ########.fr       */
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

#include "Config.hpp"
#include "Common.hpp"
#include "Logger.hpp"
#include "Worker.hpp"
#include <set>

class Worker;

class Server {
 private:
  static int                  _callCount;
  static Server*              _instance;
  Config&                     _config;
  Logger&                     _log;
  std::vector<Worker*>        _workers;
  std::map<int, ListenConfig> _listenSockets;
  pthread_mutex_t             _mutex;
  pthread_cond_t              _cond;
  int                         _epollSocket;
  int                         _activeWorkers;
  std::map<int, std::vector<VirtualServer*> > _virtualServers;


  void _setupServerSockets();
  void _setupWorkers();
  void _setupEpoll();
  void _dispatchEvent(struct epoll_event event);
  std::vector<VirtualServer*> _setupAssociateVirtualServers(
      const ListenConfig& listenConfig);

 public:

  Server();
  static Server& getInstance();
  std::vector<VirtualServer*> getVirtualServer(int fd);
  void workerFinished();
  ~Server();

  void start();
  void stop(int signum);
};

#endif
