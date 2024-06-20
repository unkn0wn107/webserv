/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/06/20 15:31:20 by mchenava         ###   ########.fr       */
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
#include "Logger.hpp"
#include "Worker.hpp"

class Worker;

class Server {
 private:
  static Server*              _instance;
  Config&                     _config;
  Logger&                     _log;
  std::vector<Worker*>        _workers;
  int                         _workerIndex;
  std::map<ListenConfig, int> _listenSockets;
  pthread_mutex_t             _mutex;
  pthread_cond_t              _cond;
  int _activeWorkers;

  void _setupServerSockets();
  void _setupWorkers();
  static void _signalHandler(int signum);


 public:
  Server();
  static Server& getInstance();
  void workerFinished();
  ~Server();

  void start();
  void stop(int signum);
};

#endif
