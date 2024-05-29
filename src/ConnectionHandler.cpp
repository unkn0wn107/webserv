/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:21 by agaley            #+#    #+#             */
/*   Updated: 2024/05/29 16:50:35 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionHandler.hpp"
#include <sys/epoll.h>
#include <algorithm>
#include <sstream>
#include <string>

ConnectionHandler::ConnectionHandler(
    int                          clientSocket,
    int                          epollSocket,
    ListenConfig&                listenConfig,
    std::vector<VirtualServer*>& virtualServers)
    : _log(Logger::getInstance()),
      _listenConfig(listenConfig),
      _connectionStatus(READING),
      _clientSocket(clientSocket),
      _epollSocket(epollSocket),
      _readn(0),
      _vservPool(virtualServers) {
  _buffer = new char[1024];
  memset(_buffer, 0, 1024);
  _log.info("CONNECTION_HANDLER: New connection handler created");
  _log.info("CONNECTION_HANDLER: serverPool size : " + _vservPool.size());
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
  bool end = false;
  while (true) {
    ssize_t bytes = recv(_clientSocket, _buffer + _readn, 1, 0);
    if (_readn >= 1024) {
      _connectionStatus = CLOSED;
      return;
    }
    if (bytes < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      } else {
        _log.error(std::string("CONNECTION_HANDLER: recv: ") + strerror(errno));
        break;
      }
    } else if (bytes == 0) {
      break;
    } else {
      _readn++;
      if (_readn >= 4 && _buffer[_readn - 1] == '\n' &&
          _buffer[_readn - 2] == '\r' && _buffer[_readn - 3] == '\n' &&
          _buffer[_readn - 4] == '\r') {
        end = true;
        break;
      }
    }
  }
  if (!end)
    return;
  _processRequest();
}

void ConnectionHandler::_sendResponse() {
  std::string response =
      "HTTP/1.1 200 OK\r\nContent-Length: 20\r\nContent-Type: "
      "text/html\r\n\r\n<h1>Hello World</h1>";
  _log.info("CONNECTION_HANDLER: Sending response");
  if (send(_clientSocket, response.c_str(), response.length(), 0) == -1) {
    _log.error(std::string("CONNECTION_HANDLER: send: ") + strerror(errno));
    _connectionStatus = CLOSED;
    return;
  }
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
  return _findDefaultServer();
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
  _request = new HTTPRequest(_buffer, _readn);
  VirtualServer* vserv = _selectVirtualServer(_request->getHost());
  if (vserv->checkRequest(*_request) != 0) {
    _log.error("CONNECTION_HANDLER: Request failed");
    _connectionStatus = CLOSED;
    return;
  }
  _log.info("CONNECTION_HANDLER: Request valid");
  _connectionStatus = SENDING;

  // if (vserv == NULL) {
  // 	_log.error("CONNECTION_HANDLER: No virtual server selected");
  // 	return;
  // }
  // _log.info(std::string("CONNECTION_HANDLER: Selected virtual server: ") +
  // vserv->getServerName()); if (vserv->parseRequest(_buffer, _readn) == -1) {
  // 	_log.error("CONNECTION_HANDLER: Request parsing failed");
  // 	_connectionStatus = CLOSED;
  // 	return;
  // }
  // _connectionStatus = SENDING;
}

void ConnectionHandler::processConnection() {
  struct epoll_event event;
  event.data.fd = _clientSocket;
  if (_connectionStatus == READING) {
    _receiveRequest();
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER: epoll_ctl: ") +
                 strerror(errno));
      return;
    }
  }
  if (_connectionStatus == SENDING) {
    _sendResponse();
    event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER: epoll_ctl: ") +
                 strerror(errno));
      return;
    }
  }
  if (_connectionStatus == CLOSED) {
    close(_clientSocket);
  }
}
