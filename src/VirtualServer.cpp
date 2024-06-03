/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 15:10:25 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/03 18:06:16 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualServer.hpp"
#include <algorithm>
#include "Utils.hpp"
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sstream>

std::map<int, std::string> VirtualServer::_defaultErrorPages;

void VirtualServer::_initializeErrorPages() {
    _defaultErrorPages[400] = "<!DOCTYPE html><html><body><h1>400 Bad Request</h1><p>Your browser sent a request that this server could not understand.</p></body></html>";
    _defaultErrorPages[403] = "<!DOCTYPE html><html><body><h1>403 Forbidden</h1><p>You don't have permission to access this resource.</p></body></html>";
    _defaultErrorPages[404] = "<!DOCTYPE html><html><body><h1>404 Not Found</h1><p>The requested resource could not be found on this server.</p></body></html>";
    _defaultErrorPages[405] = "<!DOCTYPE html><html><body><h1>405 Method Not Allowed</h1><p>The request method is not supported for the requested resource.</p></body></html>";
    _defaultErrorPages[500] = "<!DOCTYPE html><html><body><h1>500 Internal Server Error</h1><p>The server encountered an internal error and was unable to complete your request.</p></body></html>";
    _defaultErrorPages[501] = "<!DOCTYPE html><html><body><h1>501 Not Implemented</h1><p>The server does not support the functionality required to fulfill the request.</p></body></html>";
    _defaultErrorPages[502] = "<!DOCTYPE html><html><body><h1>502 Bad Gateway</h1><p>The server received an invalid response from the upstream server.</p></body></html>";
    _defaultErrorPages[503] = "<!DOCTYPE html><html><body><h1>503 Service Unavailable</h1><p>The server is currently unable to handle the request due to temporary overloading or maintenance of the server.</p></body></html>";
    _defaultErrorPages[504] = "<!DOCTYPE html><html><body><h1>504 Gateway Timeout</h1><p>The server did not receive a timely response from the upstream server.</p></body></html>";
    _defaultErrorPages[505] = "<!DOCTYPE html><html><body><h1>505 HTTP Version Not Supported</h1><p>The server does not support the HTTP protocol version used in the request.</p></body></html>";
}

VirtualServer::VirtualServer(ServerConfig& serverConfig)
    : _serverConfig(serverConfig), _log(Logger::getInstance()) {
  _hostNames = _serverConfig.server_names;
  _defaultServer = _hasDefaultListenConfig();
  _initializeErrorPages();
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

std::string VirtualServer::_generateDirectoryListing(const std::string& path) {
  DIR *dir;
  struct dirent *entry;
  struct stat statbuf;

  std::ostringstream html;
  html << "<html><body><h1>Directory listing of " << path << "</h1><ul>";

  if ((dir = opendir(path.c_str())) != NULL) {
      while ((entry = readdir(dir)) != NULL) {
          std::string entryName = entry->d_name;
          if (entryName == "." || entryName == "..")
              continue;

          std::string fullPath = path + "/" + entryName;
          if (stat(fullPath.c_str(), &statbuf) == -1) {
              continue;
          }

          html << "<li>";
          if (S_ISDIR(statbuf.st_mode)) {
              html << "[DIR] ";
          } else {
              html << "[FILE] ";
          }
          html << "<a href=\"" << URI::encode(entryName) << "\">" << entryName << "</a>";
          html << "</li>";
      }
      closedir(dir);
  } else {
      html << "<p>Error opening directory.</p>";
  }

  html << "</ul></body></html>";
  return html.str();
}

HTTPResponse*  VirtualServer::_autoindex(const std::string& path, LocationConfig& location, const std::string& uri) {
  std::string indexPath = path + "/index.html";
  std::ifstream indexFile(indexPath.c_str());
  if (indexFile && location.autoindex) {
      std::string content((std::istreambuf_iterator<char>(indexFile)), std::istreambuf_iterator<char>());
      HTTPResponse *response = new HTTPResponse(200);
      response->addHeader("Content-Type", "text/html");
      response->addHeader("Content-Length", Utils::to_string(content.size()));
      response->setBody(content);
      return response;
  } else if (location.autoindex) {
      // Générer une liste de fichiers et de répertoires
      std::string directoryListing = _generateDirectoryListing(path);
      HTTPResponse *response = new HTTPResponse(200);
      response->addHeader("Content-Type", "text/html");
      response->addHeader("Content-Length", Utils::to_string(directoryListing.size()));
      response->setBody(directoryListing);
      return response;
  } else {
      _log.error("VirtualServer::_handleGetRequest : Directory index not available");
      return new HTTPResponse(404, _getErrorPages(uri));
  }
}

std::string VirtualServer::_getPath(const std::string& uri, LocationConfig& location) {
  if (location.root.empty()) {
    return _serverConfig.root + uri;
  }
  return location.root + uri;
}

HTTPResponse  *VirtualServer::_handleGetRequest(HTTPRequest& request) {
  std::string uri = request.getURI();
  _log.info("VirtualServer::_handleGetRequest : URI : " + uri);
  LocationConfig location = _getLocationConfig(uri);
  std::string path = _getPath(uri, location);
  struct stat statbuf;
  if (stat(path.c_str(), &statbuf) == -1) {
      _log.error("VirtualServer::_handleGetRequest : Path not found: " + path);
      return new HTTPResponse(404, _getErrorPages(uri));
  }

  if (S_ISDIR(statbuf.st_mode)) {
      return _autoindex(path, location, uri);
  }
  // Gestion des fichiers normaux
  std::string contentType = HTTPResponse::getContentType(path);
  std::ifstream file(path.c_str());
  if (file) {
      std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      HTTPResponse *response = new HTTPResponse(200);
      response->addHeader("Content-Type", contentType);
      response->addHeader("Content-Length", Utils::to_string(content.size()));
      response->setBody(content);
      return response;
  }

  _log.error("VirtualServer::_handleGetRequest : File not found");
  return new HTTPResponse(404, _getErrorPages(uri));
}

std::string VirtualServer::defaultErrorPage(int status) {
  if (_defaultErrorPages.find(status) != _defaultErrorPages.end()) {
    return _defaultErrorPages[status];
  }
  return "";
}

std::string VirtualServer::_fillErrorPage(int status, const std::string& path) {
  std::ifstream file(path.c_str());
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
  // if (method == "POST") {
  //   return _handlePostRequest(request);
  // }
  // if (method == "DELETE") {
  //   return _handleDeleteRequest(request);
  // }
  // if (method == "PUT") {
  //   return _handlePutRequest(request);
  // }
  // if (method == "HEAD") {
  //   return _handleHeadRequest(request);
  // }
  // if (method == "CONNECT") {
  //   return _handleConnectRequest(request);
  // }
  // if (method == "OPTIONS") {
  //   return _handleOptionsRequest(request);
  // }
  _log.error("VirtualServer::handleRequest : Method not allowed");
  return new HTTPResponse(403, _getErrorPages(uri));
}

std::string VirtualServer::getServerName() const {
  return _serverConfig.server_names[0];
}

VirtualServer::~VirtualServer() {}
