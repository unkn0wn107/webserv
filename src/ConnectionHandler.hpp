/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:25 by agaley            #+#    #+#             */
/*   Updated: 2024/06/21 14:38:15 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <string.h>
#include <sys/socket.h>
#include <vector>
#include "Config.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "VirtualServer.hpp"
#include <pthread.h>

#define BUFFER_SIZE 16384

enum ConnectionStatus { READING, SENDING, CLOSED };

class ConnectionHandler {
 public:
  ConnectionHandler(int clientSocket,
                    int epollSocket,
                    // ListenConfig&                listenConfig,
                    std::vector<VirtualServer*>& virtualServers);
  ~ConnectionHandler();

  // HTTPProtocol* selectHTTPProtocolVersion(const std::string& requestString);
  void processConnection();
  int  getConnectionStatus();

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
  // ListenConfig&               _listenConfig;
  int                         _connectionStatus;
  int                         _clientSocket;
  int                         _epollSocket;
  char*                       _buffer;
  int                         _readn;
  std::vector<VirtualServer*> _vservPool;
  HTTPRequest*                _request;
  HTTPResponse*               _response;
  pthread_mutex_t             _mutex;

  void           _receiveRequest();
  void           _processRequest();
  VirtualServer* _selectVirtualServer(std::string host);
  VirtualServer* _findDefaultServer();
  std::string    _extractHost(const std::string& requestHeader);
  void           _sendResponse();
};

#endif
