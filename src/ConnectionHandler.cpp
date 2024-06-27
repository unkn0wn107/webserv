/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:21 by agaley            #+#    #+#             */
/*   Updated: 2024/06/27 15:28:16 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include <sys/epoll.h>
#include <algorithm>
#include <sstream>
#include <string>

#include "Common.hpp"
#include "ConnectionHandler.hpp"
#include "Utils.hpp"

ConnectionHandler::ConnectionHandler(
    int                          clientSocket,
    int                          epollSocket,
    std::vector<VirtualServer*>& virtualServers)
    : _log(Logger::getInstance()),
      _connectionStatus(READING),
      _clientSocket(clientSocket),
      _epollSocket(epollSocket),
      _buffer(new char[BUFFER_SIZE]),
      _readn(0),
      _vservPool(virtualServers),
      _request(NULL),
      _response(NULL) {
  memset(_buffer, 0, BUFFER_SIZE);
}

ConnectionHandler::~ConnectionHandler() {
  for (std::vector<VirtualServer*>::iterator it = _vservPool.begin();
       it != _vservPool.end(); ++it) {
    delete *it;
  }
  delete[] _buffer;
  if (_request) {
    delete _request;
    _request = NULL;
  }
  if (_response) {
    delete _response;
    _response = NULL;
  }
}

void ConnectionHandler::_receiveRequest() {
  bool        headersEnd = false;
  ssize_t     bytes;
  std::string headers;
  size_t      contentLength = 0;
  bool        contentLengthFound = false;
  size_t      headersEndPos = 0;
  int         trys = 0;

  while (!headersEnd && (bytes = recv(_clientSocket, _buffer + _readn,
                                      BUFFER_SIZE - _readn, 0)) > 0) {
    if (bytes <= 0) {
      if (trys > 3) {
        _log.error(std::string("CONNECTION_HANDLER: recv: ") + strerror(errno));
        _connectionStatus = CLOSED;
        return;
      }
      trys++;
      usleep(1000);
      continue;
    }
    trys = 0;

    _readn += bytes;
    headers.append(_buffer, _readn);
    headersEndPos = headers.find("\r\n\r\n");
    if (headersEndPos != std::string::npos) {
      headersEnd = true;
      size_t clPos = headers.find("Content-Length:");
      if (clPos != std::string::npos) {
        size_t      clEnd = headers.find("\r\n", clPos);
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

  if (contentLengthFound && _readn - headersEndPos - 4 != contentLength) {
    _log.error("CONNECTION_HANDLER: Content-Length mismatch");
    _connectionStatus = CLOSED;
    return;
  }
  _processRequest();
  _readn = 0;
}

void ConnectionHandler::_sendResponse() {
  if (_response->sendResponse(_clientSocket) == -1)
    throw WriteException("CONNECTION_HANDLER: Failed to send response");
  _connectionStatus = CLOSED;
}

VirtualServer* ConnectionHandler::_selectVirtualServer(std::string host) {
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
  if (_request) {
    delete _request;
    _request = NULL;
  }
  _request = new HTTPRequest(_buffer);

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

  _response = vserv->handleRequest(*_request);

  _response->setCookie("sessionid", _request->getSessionId());

  _connectionStatus = SENDING;
}

int ConnectionHandler::getConnectionStatus() {
  return _connectionStatus;
}

void ConnectionHandler::_processData() {
  if (_connectionStatus == READING) {
    try {
      _receiveRequest();
    } catch (const Exception& e) {
      _log.error(std::string("CONNECTION_HANDLER: Exception caught: ") +
                 e.what());
      return;
    }
  }
  if (_connectionStatus == SENDING) {
    try {
      _sendResponse();
    } catch (const Exception& e) {
      _log.error(std::string("CONNECTION_HANDLER: Exception caught: ") +
                 e.what());
      _connectionStatus = CLOSED;
      return;
    }
  }
}

void ConnectionHandler::processConnection() {
  std::clock_t start = std::clock();  // Timestamp de début

  struct epoll_event event;
  event.data.ptr = this;
  _processData();
  if (_connectionStatus == READING) {
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER: epoll_ctl: ") +
                 strerror(errno));
      close(_clientSocket);  // Fermer le fd en cas d'erreur
      return;
    }
  } else if (_connectionStatus == SENDING) {
    event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER: epoll_ctl: ") +
                 strerror(errno));
      close(_clientSocket);  // Fermer le fd en cas d'erreur
      return;
    }
  }
  if (_connectionStatus == CLOSED) {
    std::string host = _request->getHost();
    std::string method = _request->getMethod();
    std::string uri = _request->getURI();
    std::string protocol = _request->getProtocol();
    std::string status = Utils::to_string(_response->getStatusCode());
    std::map<std::string, std::string> headers = _response->getHeaders();
    std::map<std::string, std::string>::const_iterator it =
        headers.find("Content-Length");
    if (it != headers.end()) {
      std::string contentLength = it->second;
      _log.info("Request : " + host + " - " + method + " " + uri + " " +
                protocol);
      _log.info("Response : " + status + " sent " + contentLength + " bytes");
    } else {
      _log.info("Request : " + host + " - " + method + " " + uri + " " +
                protocol);
      _log.info("Response : " + status + " sent with no Content-Length");
    }
    epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _clientSocket, NULL);
    close(_clientSocket);
    std::clock_t end = std::clock();
    double       duration = static_cast<double>(end - start) / CLOCKS_PER_SEC;
    if (duration > 0.005) {
      _log.warning(
          "CONNECTION_HANDLER: =================================Processing "
          "time: " +
          Utils::to_string(duration) + " seconds");
    }
    delete this;
  }
}
