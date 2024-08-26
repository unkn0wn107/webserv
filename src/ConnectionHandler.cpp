/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:21 by agaley            #+#    #+#             */
/*   Updated: 2024/07/24 19:55:49 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include <sys/epoll.h>
#include <algorithm>
#include <sstream>
#include <string>

#include "Common.hpp"
#include "ConnectionHandler.hpp"
#include "Utils.hpp"

ConnectionHandler::ConnectionHandler(int                          clientSocket,
                                     int                          epollSocket,
                                     std::vector<VirtualServer*>& virtualServers,
                                     ListenConfig&                listenConfig)
    : _cacheHandler(CacheHandler::getInstance()),
      _log(Logger::getInstance()),
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
      _cgiState(NONE),
      _step(0) {}

ConnectionHandler::~ConnectionHandler() {
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
  _requestString.clear();
  _readn = 0;
}

ConnectionStatus ConnectionHandler::getConnectionStatus() const {
  return _connectionStatus;
}

void ConnectionHandler::setInternalServerError() {
  VirtualServer*        vserv = _selectVirtualServer(_request->getHost());
  std::string           uriPath = _request->getURIComponents().path;
  const LocationConfig& location = vserv->getLocationConfig(uriPath);
  _response = new HTTPResponse(HTTPResponse::INTERNAL_SERVER_ERROR, location);
  _setConnectionStatus(SENDING);
}

ConnectionStatus ConnectionHandler::_checkConnectionStatus() {
  ConnectionStatus status = _connectionStatus;
  return status;
}

void ConnectionHandler::_setConnectionStatus(ConnectionStatus status) {
  _connectionStatus = status;
}

void ConnectionHandler::_receiveRequest(struct epoll_event& event) {
  ssize_t bytes;
  char buffer[_rcvbuf];

  if (_count > MAX_TRIES || (time(NULL) - _startTime) > TIMEOUT) {
    _setConnectionStatus(CLOSED);
    return;
  }

  bytes = recv(_clientSocket, buffer, _rcvbuf, 0);
  if (bytes <= 0) {
    if (bytes == 0 || (bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) {
      _setConnectionStatus(CLOSED);
    }
    return;
  }

  _readn += bytes;
  _requestString.append(buffer, bytes);
  _log.info("CONNECTION_HANDLER: _requestString: " + _requestString);
  // Check if we've received the complete headers
  size_t headersEndPos = _requestString.find("\r\n\r\n");
  if (headersEndPos == std::string::npos) {
    return; // Headers not complete, wait for more data
  }

  // If we haven't parsed the request yet, do it now
  if (!_request)
    _request = new HTTPRequest(_requestString.substr(0, headersEndPos));

  std::string method = _request->getMethod();
  if (method == "GET" || method == "HEAD" || method == "DELETE") {
    epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _clientSocket, &event);
    _processRequest(event);
    return;
  }

  std::string contentLengthStr = _request->getHeader("Content-Length");
  if (contentLengthStr.empty()) {
    HTTPResponse::sendResponse(HTTPResponse::LENGTH_REQUIRED, _clientSocket);
    _setConnectionStatus(CLOSED);
    return;
  }

  size_t contentLength = Utils::stoi<size_t>(contentLengthStr);
  size_t bodyLength = _readn - (headersEndPos + 4); // +4 for "\r\n\r\n"

  if (bodyLength >= contentLength) {
    epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _clientSocket, &event);
    _request->setBody(_requestString.substr(headersEndPos + 4, contentLength));
    _processRequest(event);
  }
  // If bodyLength < contentLength, we need to wait for more data
}

void ConnectionHandler::_sendResponse() {
  try {
    if (_count > MAX_TRIES || (time(NULL) - _startTime) > TIMEOUT) {
      _setConnectionStatus(CLOSED);
      return;
    }
    _response->setCookie("sessionid", _request->getSessionId());
    if (_response->sendResponse(_clientSocket, _sndbuf) != 0)
      _setConnectionStatus(CLOSED);
  } catch (const Exception& e) {
    _setConnectionStatus(CLOSED);
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
        "CONNECTION_HANDLER: No matching virtual server found for host: " + host);
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

void ConnectionHandler::_processRequest(struct epoll_event& event) {
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
  std::string           uriPath = _request->getURIComponents().path;
  const LocationConfig& location = vserv->getLocationConfig(uriPath);
  if (location.cgi && CGIHandler::isScript(*_request, location) && _cgiState == NONE) {
    if (_request->getHeader("Cache-Control") != "no-cache") {
      CacheStatus cacheStatus = _cacheHandler.getCacheStatus(
          _cacheHandler.generateKey(*_request), static_cast<EventData*>(event.data.ptr));
      if (cacheStatus == CACHE_FOUND) {
        _response = new HTTPResponse(*_cacheHandler.getResponse(_cacheHandler.generateKey(*_request)));
        _setConnectionStatus(SENDING);
        return;
      } else if (cacheStatus == CACHE_CURRENTLY_BUILDING) {
        _setConnectionStatus(CACHE_WAITING);
        return;
      }
    }
    _response = new HTTPResponse(HTTPResponse::OK, location);
    _cgiHandler = new CGIHandler(*_request, *_response, _epollSocket, location);
    _cgiState = REGISTER_SCRIPT_FD;
    _setConnectionStatus(EXECUTING);
  } else {
    _response = vserv->handleRequest(*_request);
    _setConnectionStatus(SENDING);
  }
}

void ConnectionHandler::_processExecutingState() {
  if (_cgiState == NONE) {
    _log.error("CONNECTION_HANDLER(" + Utils::to_string(_step) +
               "): CGIHandler is NULL or in invalid state");
    _setConnectionStatus(SENDING);
    return;
  }
  ConnectionStatus returnStatus = _cgiHandler->handleCGIRequest();
  _setConnectionStatus(returnStatus);
  _cgiState = _cgiHandler->getState();
}

int ConnectionHandler::processConnection(struct epoll_event& event) {
  _step++;
  try {
    if (_connectionStatus == CACHE_WAITING) {
      CacheStatus cacheStatus = _cacheHandler.getCacheStatus(
          _cacheHandler.generateKey(*_request), static_cast<EventData*>(event.data.ptr));
      if (cacheStatus == CACHE_FOUND) {
        _response = new HTTPResponse(*_cacheHandler.getResponse(_cacheHandler.generateKey(*_request)));
        _setConnectionStatus(SENDING);
      }
      // else if (cacheStatus.status == CACHE_CURRENTLY_BUILDING) {
      //   _setConnectionStatus(CACHE_WAITING);
      // }
    }
    if (_connectionStatus == READING)
      _receiveRequest(event);
    if (_connectionStatus == EXECUTING)
      _processExecutingState();
    if (_connectionStatus == SENDING)
      _sendResponse();
  } catch (const Exception& e) {
    _setConnectionStatus(CLOSED);
    _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) +
                           "): " + " Status: " + getStatusString() +
                           " Exception caught: " + e.what()));
  }
  switch (_connectionStatus) {

    case READING:
      event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
      if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
        _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) +
                               "): epoll_ctl: ") +
                   strerror(errno));
        close(_clientSocket);
      }
      break;

    case EXECUTING:
      if (!_cgiHandler)
        throw Exception("CONNECTION_HANDLER(" + Utils::to_string(_step) +
                        "): CGIHandler is NULL");
      if (_cgiHandler->getCgifd() == -1)
        throw Exception("CONNECTION_HANDLER(" + Utils::to_string(_step) +
                        "): CGIHandler file descriptor is -1");
      int op;
      event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
      if (_cgiState == REGISTER_SCRIPT_FD) {
        _cgiHandler->setState(SCRIPT_RUNNING);
        op = EPOLL_CTL_ADD;
      }
      else if (_cgiState == PROCESS_OUTPUT || _cgiState == CGI_ERROR)
        op = EPOLL_CTL_DEL;
      else
        op = EPOLL_CTL_MOD;
      if (epoll_ctl(_epollSocket, op, _cgiHandler->getCgifd(), &event) == -1) {
        _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) +
                               "): epoll_ctl: ") +
                   strerror(errno));
        close(_cgiHandler->getCgifd());
      }
      break;

    case CACHE_WAITING:
      break;

    case SENDING:
      event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
      if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
        if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, _clientSocket, &event) == -1) {
          _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) +
                                "): epoll_ctl: ") +
                    strerror(errno));
          close(_clientSocket);
        }
      }
      break;

    case CLOSED:
      EventData* eventData = static_cast<EventData*>(event.data.ptr);
      _handleClosedConnection();
      eventData->opened = false;
      return 1;  // Done
  }
  return 0;
}

void ConnectionHandler::_handleClosedConnection() {
  if (!_request)
    return;
  std::string host = _request->getHost();
  std::string method = _request->getMethod();
  std::string uri = _request->getURI();
  std::string protocol = _request->getProtocol();
  std::string status = Utils::to_string(_response->getStatusCode());
  std::map<std::string, std::string>                 headers = _response->getHeaders();
  std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
  if (it != headers.end()) {
    std::string contentLength = it->second;
    _log.info("Request : " + host + " - " + method + " " + uri + " " + protocol);
    _log.info("Response : " + status + " sent " + contentLength + " bytes");
  } else {
    _log.info("Request : " + host + " - " + method + " " + uri + " " + protocol);
    _log.info("Response : " + status + " sent with no Content-Length");
  }
}

void ConnectionHandler::_cleanupCGIHandler() {
  if (_cgiState == DONE) {
    delete _cgiHandler;
    _cgiHandler = NULL;
    _cgiState = NONE;
  }
}

std::string ConnectionHandler::getStatusString() const {
  std::string statusStr;
  switch (_connectionStatus) {
    case READING:
      statusStr = "READING";
      break;
    case EXECUTING:
      statusStr = "EXECUTING";
      break;
    case SENDING:
      statusStr = "SENDING";
      break;
    case CLOSED:
      statusStr = "CLOSED";
      break;
    default:
      statusStr = "UNKNOWN";
      break;
  }
  return statusStr;
}
