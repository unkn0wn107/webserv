/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 15:10:25 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/02 23:10:18 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualServer.hpp"
#include <algorithm>
#include "Utils.hpp"

VirtualServer::VirtualServer(ServerConfig& serverConfig)
    : _serverConfig(serverConfig), _log(Logger::getInstance()) {
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

LocationConfig& VirtualServer::_getLocationConfig(const std::string& uri) {
  std::map<std::string, LocationConfig>::iterator it;
  std::map<std::string, LocationConfig>::iterator bestMatch =
      _serverConfig.locations.end();
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

int VirtualServer::checkRequest(HTTPRequest& request) {
  std::string protocol = request.getProtocol();
  std::string method = request.getMethod();
  std::string uri = request.getURI();
  _log.info("VirtualServer::checkRequest : Protocol : " + protocol +
            " Method : " + method + " URI : " + uri);
  if (protocol != "HTTP/1.1") {
    _log.error("VirtualServer::checkRequest : Protocol not supported");
    return 400;
  }
  LocationConfig location = _getLocationConfig(uri);
  _log.info("VirtualServer::checkRequest : Location : " + location.location);
  if (location.location == "") {
    _log.error("VirtualServer::checkRequest : Location not found");
    return 404;
  }
  LocationConfig defaultLocation = _getLocationConfig("/");
  _log.info("VirtualServer::checkRequest : Default location : " +
            defaultLocation.location);
  bool isMethodAllowedInLocation =
      std::find(location.allowed_methods.begin(),
                location.allowed_methods.end(),
                method) != location.allowed_methods.end();
  bool isMethodAllowedInDefaultLocation =
      std::find(defaultLocation.allowed_methods.begin(),
                defaultLocation.allowed_methods.end(),
                method) != defaultLocation.allowed_methods.end();
  if (!isMethodAllowedInLocation || !isMethodAllowedInDefaultLocation) {
    _log.error("VirtualServer::checkRequest : Method not allowed");
    return 403;  // Forbidden
  }
  return 0;
}

int VirtualServer::_handleGetRequest(HTTPRequest& request) {
  std::string uri = request.getURI();
  _log.info("VirtualServer::_handleGetRequest : URI : " + uri);
  LocationConfig location = _getLocationConfig(uri);
  _log.info("VirtualServer::_handleGetRequest : Location : " + location.location);
  
  return 0;
}

int VirtualServer::handleRequest(HTTPRequest& request) {
  std::string protocol = request.getProtocol();
  std::string method = request.getMethod();
  std::string uri = request.getURI();
  _log.info("VirtualServer::handleRequest : Protocol : " + protocol +
            " Method : " + method + " URI : " + uri);

  if (protocol != "HTTP/1.1") {
    _log.error("VirtualServer::handleRequest : Protocol not supported");
    return 400;
  }
  if (method == "GET") {
    return _handleGetRequest(request);
  }
  if (method == "POST") {
    return _handlePostRequest(request);
  }
  if (method == "DELETE") {
    return _handleDeleteRequest(request);
  }
  if (method == "PUT") {
    return _handlePutRequest(request);
  }
  if (method == "HEAD") {
    return _handleHeadRequest(request);
  }
  if (method == "CONNECT") {
    return _handleConnectRequest(request);
  }
  if (method == "OPTIONS") {
    return _handleOptionsRequest(request);
  }
  _log.error("VirtualServer::handleRequest : Method not allowed");
  return 403;
}

std::string VirtualServer::getServerName() const {
  return _serverConfig.server_names[0];
}

VirtualServer::~VirtualServer() {}
