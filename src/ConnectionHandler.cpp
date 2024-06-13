/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:21 by agaley            #+#    #+#             */
/*   Updated: 2024/06/14 00:23:16 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionHandler.hpp"
#include <sys/epoll.h>
#include <algorithm>
#include <sstream>
#include <string>
#include "Utils.hpp"

ConnectionHandler::ConnectionHandler(
    int clientSocket,
    int epollSocket,
    // ListenConfig&                listenConfig,
    std::vector<VirtualServer*>& virtualServers)
    : _log(Logger::getInstance()),
      // _listenConfig(listenConfig),
      _connectionStatus(READING),
      _clientSocket(clientSocket),
      _epollSocket(epollSocket),
      _vservPool(virtualServers) {
  _buffer = new char[BUFFER_SIZE];
  memset(_buffer, 0, BUFFER_SIZE);
  _log.info("CONNECTION_HANDLER: New connection handler created");
  _log.info("CONNECTION_HANDLER: serverPool size : " +
            Utils::to_string(_vservPool.size()));
  for (std::vector<VirtualServer*>::iterator it = _vservPool.begin();
       it != _vservPool.end(); ++it) {
    _log.info("CONNECTION_HANDLER: serverPool name : " +
              (*it)->getServerName());
  }
}

ConnectionHandler::~ConnectionHandler() {
  delete[] _buffer;
}

void ConnectionHandler::_receiveRequest() {
  std::vector<char>  buffer(BUFFER_SIZE);
  std::ostringstream requestStream;
  ssize_t            bytesRead;
  bool               headerComplete = false;

  while ((bytesRead = recv(_clientSocket, &buffer[0], buffer.size(), 0)) > 0) {
    requestStream.write(&buffer[0], bytesRead);
    if (requestStream.str().find("\r\n\r\n") != std::string::npos) {
      headerComplete = true;
      break;  // End of request header
    }
  }

  if (bytesRead == -1) {
    throw ReadException("CONNECTION_HANDLER: recv error -1");
  }

  if (!headerComplete)
    throw ReadException(
        "CONNECTION_HANDLER: Incomplete request header received.");
  _request = new HTTPRequest(requestStream.str());
  _processRequest();
}

void ConnectionHandler::_sendResponse() {
  if (_response->sendResponse(_clientSocket) == -1)
    throw WriteException("CONNECTION_HANDLER: Failed to send response");
  _connectionStatus = CLOSED;
}

VirtualServer* ConnectionHandler::_selectVirtualServer(std::string host) {
  _log.info(std::string("CONNECTION_HANDLER: Host extracted: ") + host);
  if (!host.empty()) {
    for (std::vector<VirtualServer*>::iterator it = _vservPool.begin();
         it != _vservPool.end(); ++it) {
      if ((*it)->isHostMatching(host)) {
        return *it;
      }
    }
  }
  VirtualServer* serv = _findDefaultServer();
  if (serv == NULL)
    throw ServerSelectionException(
        "CONNECTION_HANDLER: No matching virtual server found for host: " +
        host);
  return serv;
}

VirtualServer* ConnectionHandler::_findDefaultServer() {
  bool           first = true;
  VirtualServer* firstVserv = NULL;
  for (std::vector<VirtualServer*>::iterator it = _vservPool.begin();
       it != _vservPool.end(); ++it) {
    if (first) {
      firstVserv = *it;
      first = false;
    }

    if ((*it)->isDefaultServer()) {
      return *it;
    }
  }
  return firstVserv;
}

std::string ConnectionHandler::_extractHost(const std::string& requestHeader) {
  std::istringstream stream(requestHeader);
  std::string        line;
  std::string        hostPrefix = "Host: ";

  while (std::getline(stream, line)) {
    size_t pos = line.find(hostPrefix);
    if (pos != std::string::npos) {
      std::string hostValue = line.substr(pos + hostPrefix.length());
      size_t      first = hostValue.find_first_not_of(" \t");
      size_t      last = hostValue.find_last_not_of(" \r\n");
      if (first != std::string::npos && last != std::string::npos) {
        return hostValue.substr(first, (last - first + 1));
      }
      break;
    }
  }
  return "";
}

void ConnectionHandler::_processRequest() {
  // TODO ? : Initialize request with  location config and change
  // LocationConfig&
  // _request = new HTTPRequest(_buffer);
  VirtualServer* vserv = _selectVirtualServer(_request->getHost());
  if (vserv == NULL) {
    _log.error("CONNECTION_HANDLER: No virtual server selected");
    HTTPResponse::sendResponse(500, _clientSocket);
    _connectionStatus = CLOSED;
    return;
  }
  if ((_response = vserv->checkRequest(*_request)) != NULL) {
    _log.error("CONNECTION_HANDLER: Request failed");
    _connectionStatus = SENDING;
    return;
  }
  _log.info("CONNECTION_HANDLER: Request valid");
  _response = vserv->handleRequest(*_request);
  _connectionStatus = SENDING;
}

void ConnectionHandler::processConnection() {
  struct epoll_event event;
  event.data.fd = _clientSocket;
  event.data.ptr = this;
  if (_connectionStatus == READING) {
    try {
      _log.info(std::string("CONNECTION_HANDLER: receive req: "));
      _receiveRequest();
    } catch (const Exception& e) {
      _log.error(std::string("CONNECTION_HANDLER: Exception caught: ") +
                 e.what());
      return;
    }
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER: epoll_ctl: ") +
                 strerror(errno));
      return;
    }
  }
  if (_connectionStatus == SENDING) {
    try {
      _sendResponse();
    } catch (const Exception& e) {
      _log.error(
          std::string("CONNECTION_HANDLER: Exception caught while sending: ") +
          e.what());
      return;
    }
    event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER: epoll_ctl: ") +
                 strerror(errno));
      return;
    }
  }
  if (_connectionStatus == CLOSED) {
    close(_clientSocket);
    delete this;
  }
}
