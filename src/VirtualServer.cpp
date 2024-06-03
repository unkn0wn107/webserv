/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 15:10:25 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/03 10:59:59 by  mchenava        ###   ########.fr       */
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

HTTPResponse *VirtualServer::checkRequest(HTTPRequest& request) {
  std::string protocol = request.getProtocol();
  std::string method = request.getMethod();
  std::string uri = request.getURI();
  _log.info("VirtualServer::checkRequest : Protocol : " + protocol +
            " Method : " + method + " URI : " + uri);
  if (protocol != "HTTP/1.1") {
    _log.error("VirtualServer::checkRequest : Protocol not supported");
    return new HTTPResponse(400, _getErrorPages(uri));
  }
  LocationConfig location = _getLocationConfig(uri);
  _log.info("VirtualServer::checkRequest : Location : " + location.location);
  if (location.location == "") {
    _log.error("VirtualServer::checkRequest : Location not found");
    return new HTTPResponse(404, _getErrorPages(uri));
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
    return new HTTPResponse(403, _getErrorPages(uri));
  }
  return NULL;
}

HTTPResponse  *VirtualServer::_handleGetRequest(HTTPRequest& request) {
  HTTPResponse *response;
  std::string uri = request.getURI();
  _log.info("VirtualServer::_handleGetRequest : URI : " + uri);
  LocationConfig location = _getLocationConfig(uri);
  std::string path = location.root + uri;

  struct stat statbuf;
  if (stat(path.c_str(), &statbuf) == -1) {
      _log.error("VirtualServer::_handleGetRequest : Path not found");
      return new HTTPResponse(404, _getErrorPages(uri));
  }

  if (S_ISDIR(statbuf.st_mode)) {
      std::string indexPath = path + "/index.html";
      std::ifstream indexFile(indexPath);
      if (indexFile && location.autoindex) {
          std::string content((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
          HTTPResponse *response = new HTTPResponse(200);
          response->addHeader("Content-Type", "text/html");
          response->addHeader("Content-Length", std::to_string(content.size()));
          response->setBody(content);
          return response;
      } else if (location.autoindex) {
          // Générer une liste de fichiers et de répertoires
          std::string directoryListing = generateDirectoryListing(path);
          HTTPResponse *response = new HTTPResponse(200);
          response->addHeader("Content-Type", "text/html");
          response->addHeader("Content-Length", std::to_string(directoryListing.size()));
          response->setBody(directoryListing);
          return response;
      } else {
          _log.error("VirtualServer::_handleGetRequest : Directory index not available");
          return new HTTPResponse(404, _getErrorPages(uri));
      }
  }

  // Gestion des fichiers normaux
  std::string contentType = HTTPResponse::getContentType(path);
  std::ifstream file(path);
  if (file) {
      std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      HTTPResponse *response = new HTTPResponse(200);
      response->addHeader("Content-Type", contentType);
      response->addHeader("Content-Length", std::to_string(content.size()));
      response->setBody(content);
      return response;
  }

  _log.error("VirtualServer::_handleGetRequest : File not found");
  return new HTTPResponse(404, _getErrorPages(uri));
}

std::string VirtualServer::_fillErrorPage(int status, const std::string& path) {
  std::ifstream file(path);
  if (!file) {
    _log.error("VirtualServer::_fillErrorPage : File not found");
    return defaultErrorPage(status);
  }
  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  return content;
}

std::map<int, std::string> VirtualServer::_getErrorPages(const std::string& uri) {
  std::map<int, std::string> errorPages;
  LocationConfig location = _getLocationConfig(uri);
  for (std::map<int, std::string>::iterator it = _serverConfig.error_pages.begin();
      it != _serverConfig.error_pages.end(); ++it) {
    errorPages[it->first] = _fillErrorPage(it->first, it->second);
  }
  for (std::map<int, std::string>::iterator it = location.error_pages.begin();
      it != location.error_pages.end(); ++it) {
    errorPages[it->first] = _fillErrorPage(it->first, it->second);
  }
  return errorPages;
}


HTTPResponse  *VirtualServer::handleRequest(HTTPRequest& request) {
  std::string protocol = request.getProtocol();
  std::string method = request.getMethod();
  std::string uri = request.getURI();
  _log.info("VirtualServer::handleRequest : Protocol : " + protocol +
            " Method : " + method + " URI : " + uri);

  if (protocol != "HTTP/1.1") {
    _log.error("VirtualServer::handleRequest : Protocol not supported");
    return new HTTPResponse(400, _getErrorPages(uri));
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
  return new HTTPResponse(403, _getErrorPages(uri));
}

std::string VirtualServer::getServerName() const {
  return _serverConfig.server_names[0];
}

VirtualServer::~VirtualServer() {}
