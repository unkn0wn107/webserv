/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:25 by agaley            #+#    #+#             */
/*   Updated: 2024/05/24 16:34:53 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <vector>
#include <string.h>
#include <sys/socket.h>
#include "VirtualServer.hpp"
#include "Config.hpp"
#include "Logger.hpp"

enum ConnectionStatus {
  READING,
  SENDING,
  CLOSED
};

class ConnectionHandler {
 public:
  ConnectionHandler(int clientSocket, int epollSocket, ListenConfig& listenConfig, std::vector<VirtualServer*>& virtualServers);
  ~ConnectionHandler();

  // HTTPProtocol* selectHTTPProtocolVersion(const std::string& requestString);
  void processConnection();

 private:
  Logger&						            _log;
	ListenConfig&		            	_listenConfig;
  int								            _connectionStatus;
	int							            	_clientSocket;
	int							            	_epollSocket;
  int							            	_readn;
  char*     			            	_buffer;
  std::vector<VirtualServer*>   _vservPool;

  void                _receiveRequest();
  void                _processRequest();
  VirtualServer*      _selectVirtualServer();
  VirtualServer*      _findDefaultServer();
  std::string         _extractHost(const std::string& requestHeader);


};

#endif
