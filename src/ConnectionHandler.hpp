/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:25 by agaley            #+#    #+#             */
/*   Updated: 2024/05/07 09:05:45 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "CGIHandler.hpp"
#include "ErrorHandler.hpp"
#include "FileHandler.hpp"
#include "HTTP1_1.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "Server.hpp"

class Server;

class ConnectionHandler {
 public:
  ConnectionHandler(int socket);
  ~ConnectionHandler();

  // HTTPProtocol* selectHTTPProtocolVersion(const std::string& requestString);
  void process();
  void sendResponse();
  bool hasDataToSend() const;

 private:
  int          _socket;
  HTTPRequest  _request;
  HTTPResponse _response;
  std::string  _responseBuffer;
  size_t       _responseSent;
};

#endif
