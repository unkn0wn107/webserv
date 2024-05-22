/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/05/07 09:51:40 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include "ConnectionHandler.hpp"
#include "ErrorHandler.hpp"

class ConnectionHandler;

class Server {
 public:
  Server(ServerConfig& config);
  ~Server();

  void run();

 private:
  Server(const Server&);             // Prevent copy-construction
  Server& operator=(const Server&);  // Prevent assignment

  void setupServerSocket();
  void setupEpoll();
  void acceptConnection();
  void closeAllClients();

  ServerConfig& _config;
  std::set<int> _client_sockets;
  int           _epoll_fd;
  int           _server_socket;
  Logger&       _log;
};

#endif
