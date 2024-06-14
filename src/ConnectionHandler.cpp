/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:21 by agaley            #+#    #+#             */
/*   Updated: 2024/06/14 13:50:40 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionHandler.hpp"
#include "Common.hpp"
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
      _buffer(new char[BUFFER_SIZE]),
      _readn(0),
      _vservPool(virtualServers)
{
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
  bool headersEnd = false;
  ssize_t bytes;
  std::string headers;
  size_t contentLength = 0;
  bool contentLengthFound = false;
  size_t headersEndPos = 0;

  while (!headersEnd && (bytes = recv(_clientSocket, _buffer + _readn, BUFFER_SIZE - _readn, 0)) > 0) {
    _readn += bytes;
    headers.append(_buffer, _readn);
    headersEndPos = headers.find("\r\n\r\n");
    if (headersEndPos != std::string::npos) {
      headersEnd = true;
      size_t clPos = headers.find("Content-Length:");
      if (clPos != std::string::npos) {
        size_t clEnd = headers.find("\r\n", clPos);
        std::string clValue = headers.substr(clPos + 15, clEnd - (clPos + 15));
        contentLength = Utils::stoi<size_t>(clValue);
        contentLengthFound = true;
      }
    }

    if (_readn >= BUFFER_SIZE) {
      _log.error("CONNECTION_HANDLER: Buffer overflow");
      _connectionStatus = CLOSED;
      return;
    }
  }

  if (bytes <= 0) {
      _log.error(std::string("CONNECTION_HANDLER: recv: ") + strerror(errno));
      _connectionStatus = CLOSED;
      return;
  }

  if (contentLengthFound && _readn - headersEndPos - 4 != contentLength) {
      _log.error("CONNECTION_HANDLER: Content-Length mismatch");
      _connectionStatus = CLOSED;
      return;
  }

  _log.info("CONNECTION_HANDLER: Request received: " + std::string(_buffer, _readn));
  _processRequest();
  _readn = 0;
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
  _request = new HTTPRequest(_buffer);
  std::string sessionId = _request->getSessionId();
  if (sessionId.empty()) {
    sessionId = generateSessionId();
    _request->setSessionId(sessionId);
  }

  VirtualServer* vserv = _selectVirtualServer(_request->getHost());
  if (vserv == NULL) {
    _log.error("CONNECTION_HANDLER: No virtual server selected");
    HTTPResponse::sendResponse(500, _clientSocket);
    _connectionStatus = CLOSED;
    return;
  }

  if ((_response = vserv->checkRequest(*_request)) != NULL) {
    _log.error("CONNECTION_HANDLER: Request failed");
    _response->setCookie("sessionid", sessionId);
    _connectionStatus = SENDING;
    return;
  }

  _log.info("CONNECTION_HANDLER: Request valid");
  _response = vserv->handleRequest(*_request);
  _response->setCookie("sessionid", sessionId);
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
