/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:21 by agaley            #+#    #+#             */
/*   Updated: 2024/06/27 15:28:16 by agaley           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/epoll.h>
#include <algorithm>
#include <sstream>
#include <string>

#include "Common.hpp"
#include "ConnectionHandler.hpp"
#include "Utils.hpp"

const int    ConnectionHandler::MAX_TRIES = 10;
const time_t ConnectionHandler::TIMEOUT = 10;

CacheHandler& ConnectionHandler::_cacheHandler = CacheHandler::getInstance();

ConnectionHandler::ConnectionHandler(
    int                          clientSocket,
    int                          epollSocket,
    std::vector<VirtualServer*>& virtualServers,
    ListenConfig&                listenConfig,
    EventQueue&                  events)
    : _log(Logger::getInstance()),
      _events(events),
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
      _step(0) {
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
  // _requestString.clear();
  // _readn = 0;
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

void ConnectionHandler::forceSendResponse() {
  _sendResponse();
}

ConnectionStatus ConnectionHandler::_checkConnectionStatus() {
  ConnectionStatus status = _connectionStatus;
  return status;
}

void ConnectionHandler::_setConnectionStatus(ConnectionStatus status) {
  _log.info("CONNECTION_HANDLER: Setting connection status to: " +
            getStatusString());
  _connectionStatus = status;
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
      std::string clValue =
          _requestString.substr(clPos + 15, clEnd - (clPos + 15));
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
    _processRequest();
  } else if (bytes == 0)
    throw RequestException(
        "CONNECTION_HANDLER: Request invalid: no more to read but headers not "
        "ended. Headers: " +
        _requestString);
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
  std::string           uriPath = _request->getURIComponents().path;
  const LocationConfig& location = vserv->getLocationConfig(uriPath);
  if (location.cgi && CGIHandler::isScript(*_request, location) &&
      _cgiState == NONE) {
    bool hitCache = _request->getHeader("Cache-Control") != "no-cache";
    _log.info("CONNECTION_HANDLER: CGI request detected");
    hitCache = false; // TODO: remove this line after testing cache
    if (hitCache) {
      _log.info("CONNECTION_HANDLER: Cache hit");
      CacheStatus cacheStatus = _cacheHandler.checkCache(_requestString);
      if (cacheStatus == CACHE_FOUND) {
        _log.error("CONNECTION_HANDLER: Cache found");
        _response = _cacheHandler.getResponse(_requestString);
        _setConnectionStatus(SENDING);
        return;
      }
      else if (cacheStatus == CACHE_CURRENTLY_BUILDING) {
        _log.info("CONNECTION_HANDLER: Cache currently building");
        _response = _cacheHandler.waitResponse(_requestString);
        _setConnectionStatus(SENDING);
        return;
      }
    } 
    _response = new HTTPResponse(HTTPResponse::OK, location);
    _cgiHandler =
        new CGIHandler(*_request, *_response, _epollSocket, location, this);
    _cgiState = INIT;
    _setConnectionStatus(EXECUTING);
  } else {
    _log.info("CONNECTION_HANDLER: NO CGI in request detected");
    _response = vserv->handleRequest(*_request);
    _response->setCookie("sessionid", _request->getSessionId());
    // _cacheHandler.storeResponse(*_request, *_response);
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
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) +
            "): " + " Status: " + getStatusString() + " Processing data");
  try {
    ConnectionStatus returnStatus = _cgiHandler->handleCGIRequest();
    _setConnectionStatus(returnStatus);
  } catch (const Exception& e) {
    _log.error("CONNECTION_HANDLER(" + Utils::to_string(_step) +
               "): Exception caught: " + e.what());
    _setConnectionStatus(SENDING);
  }
  _cgiState = _cgiHandler->getCgiState();
}

int ConnectionHandler::processConnection(struct epoll_event& event) {
  _step++;
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) +
            "): _rcvbuf: " + Utils::to_string(_rcvbuf));
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) +
            "): _sndbuf: " + Utils::to_string(_sndbuf));
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) +
            "): Connection status: " + getStatusString());
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) +
            "): crrent read request: [" + _requestString +
            "]\nReadn: " + Utils::to_string(_readn));

  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) +
            "): Status: " + getStatusString());
  try {
    if (_connectionStatus == READING)
      _receiveRequest();
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
  _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) +
            "): Modifying epoll events for " + getStatusString());
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
      if (!_cgiHandler) {
        throw Exception("CONNECTION_HANDLER(" + Utils::to_string(_step) +
                       "): CGIHandler is NULL");
      }
      _log.warning("CONNECTION_HANDLER(" + Utils::to_string(_step) +
                "): CGIHandler is executing, go back in event loop");
      event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
      _events.push(event);
      break;

    case SENDING:
      event.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
      if (epoll_ctl(_epollSocket, EPOLL_CTL_MOD, _clientSocket, &event) == -1) {
        _log.error(std::string("CONNECTION_HANDLER(" + Utils::to_string(_step) +
                               "): epoll_ctl: ") +
                   strerror(errno));
        close(_clientSocket);
      }
      break;

    case CLOSED:
      _handleClosedConnection();
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
  closeClientSocket();
}

void ConnectionHandler::closeClientSocket() {
  // if (fcntl(_clientSocket, F_GETFD) != -1 || errno != EBADF) {
  //   if (epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _clientSocket, NULL) == -1)
  //     _log.error("CONNECTION_HANDLER(" + Utils::to_string(_step) +
  //                "): Failed to delete client socket from epoll : " +
  //                Utils::to_string(_clientSocket) + " " +
  //                std::string(strerror(errno)));
  //   if (close(_clientSocket) == -1) {
  //     _log.error(
  //         "CONNECTION_HANDLER(" + Utils::to_string(_step) +
  //         "): Error closing client socket: " + std::string(strerror(errno)));
  //   } else {
  //     _log.info("CONNECTION_HANDLER(" + Utils::to_string(_step) +
  //               "): Client socket closed successfully");
  //   }
  // } else {
  //   _log.error("CONNECTION_HANDLER(" + Utils::to_string(_step) +
  //              "): Invalid client socket: " + Utils::to_string(_clientSocket));
  // }
  epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _clientSocket, NULL);
  close(_clientSocket);
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
