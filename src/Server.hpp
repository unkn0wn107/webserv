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
#include <string>
#include <set>
#include "ConnectionHandler.hpp"
#include "ErrorHandler.hpp"
#include "HTTPProtocol.hpp"

class ConnectionHandler;

class Server {
 public:
  Server(ServerConfig& config);
  ~Server();

  void processConnection(char *buffer);
  bool isForMe(std::string host);

 private:
  Server(const Server&);             // Prevent copy-construction
  Server& operator=(const Server&);  // Prevent assignment

  HTTPProtocol* _getProtocol(char *buffer);
  // void sendResponse();
  // bool hasDataToSend() const;

  ServerConfig&                      _config;
  Logger&                            _log;
  HTTPRequest  _request;
  HTTPResponse _response;
  std::string  _responseBuffer;
  size_t       _responseSent;
};

#endif
