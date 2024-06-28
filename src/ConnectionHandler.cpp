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

const int ConnectionHandler::MAX_TRIES = 10;
const time_t ConnectionHandler::TIMEOUT = 10;

ConnectionHandler::ConnectionHandler(
    int                          clientSocket,
    int                          epollSocket,
    std::vector<VirtualServer*>& virtualServers,
    ListenConfig               listenConfig)
    : _log(Logger::getInstance()),
      _connectionStatus(READING),
      _clientSocket(clientSocket),
      _epollSocket(epollSocket),
      _buffer(new char[BUFFER_SIZE]),
      _rcvbuf(listenConfig.rcvbuf),
      _sndbuf(listenConfig.sndbuf),
      _requestString(""),
      _readn(0),
      _vservPool(virtualServers),
      _request(NULL),
      _response(NULL),
      _count(0),
      _startTime(time(NULL)) {
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

  if (_count > MAX_TRIES || (time(NULL) - _startTime) > TIMEOUT) {
    _connectionStatus = CLOSED;
    return;
  }

  char buffer[1024];
  bytes = recv(_clientSocket, buffer, _rcvbuf, 0);
  if (bytes < 0) {
    _log.warning("CONNECTION_HANDLER: recv failed");
    return;
  }
  _readn += bytes;
  _requestString.append(buffer, bytes);
  headersEndPos = _requestString.find("\r\n\r\n");
  if (headersEndPos != std::string::npos) {
    headersEnd = true;
    _log.info("CONNECTION_HANDLER: Headers end found at position: " +
              Utils::to_string(headersEndPos));
    size_t clPos = _requestString.find("Content-Length:");
    if (clPos != std::string::npos) {
      size_t      clEnd = _requestString.find("\r\n", clPos);
      std::string clValue = _requestString.substr(clPos + 15, clEnd - (clPos + 15));
      contentLength = Utils::stoi<size_t>(clValue);
      contentLengthFound = true;
    }
  }

  if (contentLengthFound && _readn - headersEndPos - 4 != contentLength) {
    _log.error("CONNECTION_HANDLER: Content-Length mismatch");
    _connectionStatus = CLOSED;
    return;
  }
  if (headersEnd) {
    _log.info("CONNECTION_HANDLER: Request valid: no more to read but headers ended");
    _processRequest();
  } else if (bytes == 0)
    _log.error("CONNECTION_HANDLER: Request invalid: no more to read but headers not ended");
}

void ConnectionHandler::_sendResponse() {
  try {
    if (_count > MAX_TRIES || (time(NULL) - _startTime) > TIMEOUT) {
      _connectionStatus = CLOSED;
      return;
    }
    if (_response->sendResponse(_clientSocket, _sndbuf) == 1)
      _connectionStatus = CLOSED;
  } catch (const Exception& e) {
    _log.error(std::string("CONNECTION_HANDLER: Exception caught: ") +
               e.what());
    throw WriteException("CONNECTION_HANDLER: Failed to send response");
    _connectionStatus = CLOSED;
  }
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
  _request = new HTTPRequest(_requestString);
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
  std::string    uriPath = _request->getURIComponents().path;
  LocationConfig location = vserv->getLocationConfig(uriPath);
  _log.info("CONNECTION_HANDLER: Location: " + location.location);
  _log.info("CONNECTION_HANDLER: root: " + location.root);
  _request->setConfig(&location);
  LocationConfig *location2 = _request->getConfig();
  _log.info("CONNECTION_HANDLER: Location2: " + location2->location);
  _log.info("CONNECTION_HANDLER: root2: " + location2->root);
  if (location.cgi && CGIHandler::isScript(*_request) &&
      _cgiHandler == NULL) {  // CGI
    _response = new HTTPResponse(HTTPResponse::OK);
    _cgiHandler = new CGIHandler(*_request, *_response);
    _connectionStatus = EXECUTING;
  } else {  // NO CGI
    _response = vserv->handleRequest(*_request);
    _response->setCookie("sessionid", _request->getSessionId());
    _connectionStatus = SENDING;
  }
}

void ConnectionHandler::_processData() {
  if (_connectionStatus == READING) {
    _log.info("CONNECTION_HANDLER: Processing data in READING state");
    try {
      _receiveRequest();
    } catch (const Exception& e) {
      _log.error(std::string("CONNECTION_HANDLER: Exception caught: ") +
                 e.what());
      return;
    }
  }
  if (_connectionStatus == EXECUTING) {
    _log.info("CONNECTION_HANDLER: Processing data in EXECUTING state");
    _connectionStatus = _cgiHandler->handleCGIRequest();
    _log.info("CONNECTION_HANDLER: Connection status after EXECUTING: " +
              Utils::to_string(_connectionStatus));
  }
  if (_connectionStatus == SENDING) {
    _log.info("CONNECTION_HANDLER: Processing data in SENDING state");
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
  _log.info("CONNECTION_HANDLER: _rcvbuf: " + Utils::to_string(_rcvbuf));
  _log.info("CONNECTION_HANDLER: _sndbuf: " + Utils::to_string(_sndbuf));
  _log.info("CONNECTION_HANDLER: Connection status: " +
            Utils::to_string(_connectionStatus));
  _log.info("CONNECTION_HANDLER: crrent read request: [" + _requestString +
            "]\nReadn: " + Utils::to_string(_readn));
  if (_connectionStatus == READING || _connectionStatus == EXECUTING) {
    _log.info("CONNECTION_HANDLER: Modifying epoll events for READING");
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER: epoll_ctl: ") +
                 strerror(errno));
      close(_clientSocket);
      return;
    }
  } else if (_connectionStatus == SENDING) {
    _log.info("CONNECTION_HANDLER: Modifying epoll events for SENDING");
    event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER: epoll_ctl: ") +
                 strerror(errno));
      close(_clientSocket);
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
  // _log.info("CONNECTION_HANDLER: Connection Processed");
}
