/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 15:10:25 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/14 00:22:22 by agaley           ###   ########lyon.fr   */
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
  std::string    protocol = request.getProtocol();
  std::string    method = request.getMethod();
  std::string    uri = request.getURI();
  unsigned int   contentLength = request.getContentLength();
  LocationConfig location = getLocationConfig(uri);
  _log.info("VirtualServer::checkRequest : Protocol : " + protocol +
            " Method : " + method + " URI : " + uri +
            " Content Length : " + Utils::to_string(contentLength));
  if (protocol != "HTTP/1.1") {
    _log.error("VirtualServer::checkRequest : Protocol not supported");
    return new HTTPResponse(400, location.error_pages);
  }
  _log.info("VirtualServer::checkRequest : Location : " + location.location);
  if (location.location == "") {
    _log.error("VirtualServer::checkRequest : Location not found");
    return new HTTPResponse(404, location.error_pages);
  }
  bool isMethodAllowedInLocation =
      std::find(location.allowed_methods.begin(),
                location.allowed_methods.end(),
                method) != location.allowed_methods.end();
  if (!isMethodAllowedInLocation) {
    _log.error("VirtualServer::checkRequest : Method not allowed");
    return new HTTPResponse(403, location.error_pages);
  }
  // TODO : To fix
  // if (contentLength > location.client_max_body_size ||
  //     contentLength > request.getBody().length()) {
  //   _log.error("VirtualServer::checkRequest : Content length too big");
  //   return new HTTPResponse(413, location.error_pages);
  // }
  if (contentLength > location.client_max_body_size) {
    _log.error("VirtualServer::checkRequest : Content length too big");
    return new HTTPResponse(413, location.error_pages);
  }
  if (location.returnCode >= 300 && location.returnCode < 400)
    return new HTTPResponse(location.returnCode, location, location.returnUrl);

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

VirtualServer::~VirtualServer() {
  delete _httpMethods;
}
