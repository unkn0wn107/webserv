/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/05/29 18:29:41 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include "Config.hpp"
#include "Logger.hpp"
#include "Worker.hpp"

class Worker;

class Server {
 private:
  Config&                     _config;
  Logger&                     _log;
  std::vector<Worker*>        _workers;
  int                         _workerIndex;
  std::map<ListenConfig, int> _listenSockets;
  pthread_mutex_t             _epollMutex;

  void _setupServerSockets();
  void _setupWorkers();

 public:
  Server();
  ~Server();
};

#endif
