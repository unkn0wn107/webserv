/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 15:10:25 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/18 17:32:18 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualServer.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "Utils.hpp"

VirtualServer::VirtualServer(ServerConfig& serverConfig)
    : _serverConfig(serverConfig),
      _log(Logger::getInstance()),
      _httpMethods(new HTTPMethods(*this)) {
  _hostNames = _serverConfig.server_names;
  _defaultServer = _hasDefaultListenConfig();
}

bool VirtualServer::_hasDefaultListenConfig() {
  for (std::vector<ListenConfig>::const_iterator it =
           _serverConfig.listen.begin();
       it != _serverConfig.listen.end(); ++it) {
    if (it->default_server) {
      return true;
    }
  }
  return false;
}

bool VirtualServer::isDefaultServer() {
  return _defaultServer;
}

bool VirtualServer::isHostMatching(const std::string& host) const {
  for (std::vector<std::string>::const_iterator it = _hostNames.begin();
       it != _hostNames.end(); ++it) {
    if (*it == host) {
      return true;
    }
  }
  return false;
}

LocationConfig& VirtualServer::getLocationConfig(const std::string& uri) {
  std::map<std::string, LocationConfig>::iterator it;
  std::map<std::string, LocationConfig>::iterator bestMatch =
      _serverConfig.locations.find("/");
  size_t longestMatchLength = 0;

  for (it = _serverConfig.locations.begin();
       it != _serverConfig.locations.end(); ++it) {
    if (uri.compare(0, it->first.length(), it->first) == 0) {
      if (it->first.length() > longestMatchLength) {
        longestMatchLength = it->first.length();
        bestMatch = it;
      }
    }
  }

  if (bestMatch != _serverConfig.locations.end())
    return bestMatch->second;

  throw std::runtime_error("No matching location found for URI: " + uri);
}

HTTPResponse* VirtualServer::checkRequest(HTTPRequest& request) {
  std::string protocol = request.getProtocol();
  std::string method = request.getMethod();
  std::string uri = request.getURI();
  std::string body = request.getBody();
  int contentLength = request.getContentLength();
  LocationConfig location = getLocationConfig(uri);

  _log.info("Checking request: Protocol: " + protocol + ", Method: " + method +
            ", URI: " + uri + ", Content Length: " + Utils::to_string(contentLength));

  if (protocol != "HTTP/1.1") {
    _log.error("Unsupported protocol");
    return new HTTPResponse(HTTPResponse::BAD_REQUEST, location.error_pages);
  }

  if (location.location.empty()) {
    _log.error("Location not found for URI: " + uri);
    return new HTTPResponse(HTTPResponse::NOT_FOUND, location.error_pages);
  }

  if (std::find(location.allowed_methods.begin(), location.allowed_methods.end(), method) == location.allowed_methods.end()) {
    _log.error("Method not allowed for this location");
    return new HTTPResponse(HTTPResponse::METHOD_NOT_ALLOWED, location.error_pages);
  }

  if (!body.empty() && contentLength == -1) {
    _log.error("Content length not provided");
    return new HTTPResponse(HTTPResponse::LENGTH_REQUIRED, location.error_pages);
  }
  if (contentLength != -1 && ((size_t)contentLength > location.client_max_body_size || (size_t)contentLength != body.length())) {
    _log.error("Content length too big or mismatch");
    return new HTTPResponse(HTTPResponse::REQUEST_ENTITY_TOO_LARGE, location.error_pages);
  }

  if (location.returnCode >= 300 && location.returnCode < 400) {
    return new HTTPResponse(location.returnCode, location, location.returnUrl);
  }

  return NULL;
}

std::map<int, std::string> VirtualServer::_getErrorPages(
    const std::string& uri) {
  std::map<int, std::string> errorPages;
  LocationConfig             location = getLocationConfig(uri);
  for (std::map<int, std::string>::iterator it = location.error_pages.begin();
       it != location.error_pages.end(); ++it) {
    errorPages[it->first] = it->second;
  }
  return errorPages;
}

HTTPResponse* VirtualServer::handleRequest(HTTPRequest& request) {
  return _httpMethods->handleRequest(request);
}

std::string VirtualServer::getServerName() const {
  return _serverConfig.server_names[0];
}

std::string VirtualServer::getRoot() const {
  return _serverConfig.root;
}

void VirtualServer::storeSessionData(const std::string& sessionId,
                                    const std::string& key,
                                    const std::string& value) {
  _sessionStore[sessionId][key] = value;
}

std::string VirtualServer::getSessionData(const std::string& sessionId,
                                          const std::string& key) {
  return _sessionStore[sessionId][key];
}

void VirtualServer::deleteSessionData(const std::string& sessionId,
                                      const std::string& key) {
  _sessionStore[sessionId].erase(key);
}

void VirtualServer::clearSessionData(const std::string& sessionId) {
  _sessionStore[sessionId].clear();
}

VirtualServer::~VirtualServer() {
  delete _httpMethods;
}
