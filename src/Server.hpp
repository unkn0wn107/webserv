/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/07/04 02:40:57 by agaley           ###   ########lyon.fr   */
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

#include "Common.hpp"
#include "Config.hpp"
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
  std::map<int, ListenConfig>                 _listenSockets;
  int                                         _epollSocket;
  bool                                        _running;
  std::map<int, std::vector<VirtualServer*> > _virtualServers;

  void _setupServerSockets();
  void _setupEpoll();

 public:
  Server();
  static Server& getInstance();
  ~Server();

  void run();
  void stop(int signum);
};

#endif
