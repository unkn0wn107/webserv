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
    ListenConfig&              listenConfig)
    : _log(Logger::getInstance()),
      _connectionStatus(READING),
      _clientSocket(clientSocket),
      _epollSocket(epollSocket),
      _rcvbuf(listenConfig.rcvbuf),
      _sndbuf(listenConfig.sndbuf),
      _requestString(""),
      _readn(0),
      _vservPool(virtualServers),
      _request(NULL),
      _response(NULL),
      _count(0),
      _startTime(time(NULL)),
      _cgiHandler(NULL),
      _step(0) {
  if (pthread_mutex_init(&_mutex, NULL) != 0)
    _log.error("CONNECTION_HANDLER: Failed to create mutex");
  if (pthread_mutex_init(&_statusMutex, NULL) != 0)
    _log.error("CONNECTION_HANDLER: Failed to create mutex");
}

ConnectionHandler::~ConnectionHandler() {
  _log.info("CONNECTION_HANDLER: Destroying connection handler");
  for (std::vector<VirtualServer*>::iterator it = _vservPool.begin();
       it != _vservPool.end(); ++it) {
    delete *it;
  }
  if (_request) {
    delete _request;
    _request = NULL;
  }
  if (_response) {
    delete _response;
    _response = NULL;
  }
  if (_cgiHandler) {
    delete _cgiHandler;
    _cgiHandler = NULL;
  }
  // pthread_mutex_unlock(&_mutex);
  pthread_mutex_destroy(&_mutex);
  // pthread_mutex_unlock(&_statusMutex);
  pthread_mutex_destroy(&_statusMutex);
  // _requestString.clear();
  // _readn = 0;
}

int ConnectionHandler::_checkConnectionStatus() {
  pthread_mutex_lock(&_statusMutex);
  int status = _connectionStatus;
  pthread_mutex_unlock(&_statusMutex);
  return status;
}

void ConnectionHandler::_setConnectionStatus(int status) {
  pthread_mutex_lock(&_statusMutex);
  _connectionStatus = status;
  pthread_mutex_unlock(&_statusMutex);
}

void ConnectionHandler::_receiveRequest() {
  bool        headersEnd = false;
  ssize_t     bytes;
  std::string headers;
  size_t      contentLength = 0;
  bool        contentLengthFound = false;
  size_t      headersEndPos = 0;

  if (_count > MAX_TRIES || (time(NULL) - _startTime) > TIMEOUT) {
    _setConnectionStatus(CLOSED);
    return;
  }

  char* buffer = new char[_rcvbuf];
  bytes = recv(_clientSocket, buffer, _rcvbuf, 0);
  if (bytes < 0) {
    _log.warning("CONNECTION_HANDLER: recv failed");
    return;
  }
  _readn += bytes;
  _requestString.append(buffer, bytes);
  delete[] buffer;
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
    _setConnectionStatus(CLOSED);
    _log.error("CONNECTION_HANDLER: Content-Length mismatch");
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
      _setConnectionStatus(CLOSED);
      return;
    }
    if (_response->sendResponse(_clientSocket, _sndbuf) == 1)
      _setConnectionStatus(CLOSED);
  } catch (const Exception& e) {
    _setConnectionStatus(CLOSED);
    _log.error(std::string("CONNECTION_HANDLER: Exception caught: ") +
               e.what());
    throw WriteException("CONNECTION_HANDLER: Failed to send response");
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
  _log.info("CONNECTION_HANDLER: Processing request");
  if (_request) {
    delete _request;
    _request = NULL;
  }
  _request = new HTTPRequest(_requestString);
  VirtualServer* vserv = _selectVirtualServer(_request->getHost());
  if (vserv == NULL) {
    _setConnectionStatus(CLOSED);
    _log.error("CONNECTION_HANDLER: No virtual server selected");
    HTTPResponse::sendResponse(500, _clientSocket);
    return;
  }

  if ((_response = vserv->checkRequest(*_request)) != NULL) {
    _log.error("CONNECTION_HANDLER: Request failed");
    _setConnectionStatus(SENDING);
    return;
  }
  std::string    uriPath = _request->getURIComponents().path;
  LocationConfig location = vserv->getLocationConfig(uriPath);
  if (location.cgi && CGIHandler::isScript(*_request, location) &&
      _cgiHandler == NULL) {  // CGI
    _log.info("CONNECTION_HANDLER: CGI detected");
    _response = new HTTPResponse(HTTPResponse::OK, location);
    _cgiHandler = new CGIHandler(*_request, *_response, _epollSocket, location, this);
    _setConnectionStatus(EXECUTING);
  } else {  // NO CGI
    _log.info("CONNECTION_HANDLER: NO CGI detected");
    _response = vserv->handleRequest(*_request);
    _response->setCookie("sessionid", _request->getSessionId()); //TODO SET COOKIE
    _setConnectionStatus(SENDING);
  }
}

void ConnectionHandler::_processData() {
  int status = _checkConnectionStatus();
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Status: " + Utils::to_string(status));
  if (status == READING) {
    // pthread_mutex_lock(&_mutex);
    _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Processing data in READING state");
    try {
      _receiveRequest();
      status = _checkConnectionStatus();
    } catch (const Exception& e) {
      _setConnectionStatus(CLOSED);
      _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Exception caught: ") +
                 e.what());
      // pthread_mutex_unlock(&_mutex);
      return;
    }
    // pthread_mutex_unlock(&_mutex);
  }
  if (status == EXECUTING) {
    // pthread_mutex_lock(&_mutex);
    // if (_cgiHandler == NULL) {
    //   _log.error("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): CGIHandler is NULL");
    //   _setConnectionStatus(SENDING);
    //   status = _checkConnectionStatus();
    //   return;
    // }
    _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Processing data in EXECUTING state");
    _setConnectionStatus(_cgiHandler->handleCGIRequest());
    status = _checkConnectionStatus();
    if (status == SENDING) {
      _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Sending after CGI");
      epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _cgiHandler->getCgifd(), NULL);
      close(_cgiHandler->getCgifd());
      delete _cgiHandler;
      _cgiHandler = NULL;
      // pthread_mutex_unlock(&_mutex);
      return;
    }
    // pthread_mutex_unlock(&_mutex);
  }
  if (status == SENDING) {
    _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Processing data in SENDING state");
    try {
      _sendResponse();
      status = _checkConnectionStatus();
    } catch (const Exception& e) {
      _setConnectionStatus(CLOSED);
      _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Exception caught: ") +
                 e.what());
      return;
    }
  }
}

int ConnectionHandler::processConnection() {
  std::clock_t start = std::clock();  // Timestamp de début

  struct epoll_event event;
  event.data.ptr = this;
  _step++;
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): _rcvbuf: " + Utils::to_string(_rcvbuf));
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): _sndbuf: " + Utils::to_string(_sndbuf));
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Connection status: " +
            Utils::to_string(_checkConnectionStatus()));
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): crrent read request: [" + _requestString +
            "]\nReadn: " + Utils::to_string(_readn));
  _processData();
  if (_checkConnectionStatus() == READING) {
    _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Modifying epoll events for READING");
    event.events = EPOLLIN | EPOLLET  ;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): epoll_ctl: ") +
                 strerror(errno));
      close(_clientSocket);
      return 1;
    }
  }
  else if (_checkConnectionStatus() == EXECUTING) {
    _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Modifying epoll events for EXECUTING");
    event.events = EPOLLIN | EPOLLET  ;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _cgiHandler->getCgifd(),
                 &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): epoll_ctl: ") +
                 strerror(errno));
      close(_clientSocket);
      return 1;
    }
  }
  else if (_checkConnectionStatus() == SENDING) {
    _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Modifying epoll events for SENDING : " +
              Utils::to_string(_clientSocket));
    event.events = EPOLLOUT | EPOLLET  ;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
      _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): epoll_ctl: ") +
                 strerror(errno));
      close(_clientSocket);
      return 1;
    }
    _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): epoll_ctl: EPOLLOUT | EPOLLET  ");
  }
  else if (_checkConnectionStatus() == CLOSED) {
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
    std::clock_t end = std::clock();
    double       duration = static_cast<double>(end - start) / CLOCKS_PER_SEC;
    if (duration > 0.005) {
      _log.warning(
          "CONNECTION_HANDLER: =================================Processing "
          "time: " +
          Utils::to_string(duration) + " seconds");
    }
    _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) + "): Deleting connection handler at address: " + Utils::to_string(reinterpret_cast<uintptr_t>(this)));
    epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _clientSocket, NULL);
    close(_clientSocket);
    delete this;
    return 0;
  }
  return 1;
}
