/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 17:55:28 by agaley           ###   ########lyon.fr   */
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
#include <string>
#include <vector>
#include "ConnectionHandler.hpp"
#include "ErrorHandler.hpp"

class ConnectionHandler;

class Server {
 public:
  Server();
  explicit Server(std::map<std::string, std::string> config);
  ~Server();

  void start();

 private:
  Server(const Server&);             // Prevent copy-construction
  Server& operator=(const Server&);  // Prevent assignment

  void setupServerSocket();
  void setupEpoll();
  void run();
  void acceptConnection();

  std::map<std::string, std::string> _config;
  std::vector<ConnectionHandler*>    _connections;
  int                                _epoll_fd;
  int                                _server_socket;
};

#endif
