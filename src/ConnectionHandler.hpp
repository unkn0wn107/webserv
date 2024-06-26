/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:25 by agaley            #+#    #+#             */
/*   Updated: 2024/06/28 01:52:32 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <ctime>
#include <vector>

#include "Config.hpp"
#include "Common.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "VirtualServer.hpp"
#include "CGIHandler.hpp"

#define BUFFER_SIZE 16384



class VirtualServer;
class CGIHandler;

class ConnectionHandler {
 public:
 static const int    MAX_TRIES;
 static const time_t TIMEOUT;

  ConnectionHandler(int clientSocket,
                    int epollSocket,
                    std::vector<VirtualServer*>& virtualServers,
                    ListenConfig listenConfig);
  ~ConnectionHandler();
  void processConnection();

  class ConnectionException : public Exception {
   public:
    ConnectionException(const std::string& message) : Exception(message) {}
  };

  class ReadException : public ConnectionException {
   public:
    ReadException(const std::string& message) : ConnectionException(message) {}
  };

  class WriteException : public ConnectionException {
   public:
    WriteException(const std::string& message) : ConnectionException(message) {}
  };

  class ServerSelectionException : public ConnectionException {
   public:
    ServerSelectionException(const std::string& message)
        : ConnectionException(message) {}
  };

 private:
  Logger& _log;
  int                         _connectionStatus;
  int                         _clientSocket;
  int                         _epollSocket;
  char*                       _buffer;
  size_t                         _rcvbuf;
  size_t                         _sndbuf;
  std::string                 _requestString;
  size_t                         _readn;
  std::vector<VirtualServer*> _vservPool;
  HTTPRequest*                _request;
  HTTPResponse*               _response;
  int                         _count;
  time_t                      _startTime;
  CGIHandler*                 _cgiHandler;

  void           _receiveRequest();
  void           _processRequest();
  VirtualServer* _selectVirtualServer(std::string host);
  VirtualServer* _findDefaultServer();
  std::string    _extractHost(const std::string& requestHeader);
  void           _sendResponse();
  void           _processData();
};

#endif
