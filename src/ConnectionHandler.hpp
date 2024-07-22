/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:25 by agaley            #+#    #+#             */
/*   Updated: 2024/07/22 17:48:56 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <ctime>
#include <vector>

#include "CGIHandler.hpp"
#include "CacheHandler.hpp"
#include "Common.hpp"
#include "Config.hpp"
#include "EventQueue.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "VirtualServer.hpp"

#define BUFFER_SIZE 16384

class VirtualServer;
class CGIHandler;

class ConnectionHandler {
 public:
  ConnectionStatus    getConnectionStatus() const;
  std::string         getStatusString() const;
  int                 processConnection(struct epoll_event& event);
  void                setInternalServerError();
  static const int    MAX_TRIES;
  static const time_t TIMEOUT;

  ConnectionHandler(int                          clientSocket,
                    int                          epollSocket,
                    std::vector<VirtualServer*>& virtualServers,
                    ListenConfig&                listenConfig);
  ~ConnectionHandler();

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
    ServerSelectionException(const std::string& message) : ConnectionException(message) {}
  };

  class RequestException : public ConnectionException {
   public:
    RequestException(const std::string& message) : ConnectionException(message) {}
  };

 private:
  CacheHandler&               _cacheHandler;
  Logger&                     _log;
  ConnectionStatus            _connectionStatus;
  int                         _clientSocket;
  int                         _epollSocket;
  ssize_t                     _rcvbuf;
  ssize_t                     _sndbuf;
  std::string                 _requestString;
  size_t                      _readn;
  std::vector<VirtualServer*> _vservPool;
  HTTPRequest*                _request;
  HTTPResponse*               _response;
  int                         _count;
  time_t                      _startTime;
  CGIHandler*                 _cgiHandler;
  CGIState                    _cgiState;
  int                         _step;

  void             _receiveRequest(struct epoll_event& event);
  void             _processRequest(struct epoll_event& event);
  VirtualServer*   _selectVirtualServer(std::string host);
  VirtualServer*   _findDefaultServer();
  std::string      _extractHost(const std::string& requestHeader);
  void             _sendResponse();
  ConnectionStatus _checkConnectionStatus();
  void             _setConnectionStatus(ConnectionStatus status);

  void _processExecutingState();
  void _cleanupCGIHandler();

  void _handleClosedConnection();
};

#endif
