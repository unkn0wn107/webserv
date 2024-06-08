/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 15:10:25 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/07 16:29:31 by agaley           ###   ########lyon.fr   */
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

HTTPResponse* VirtualServer::checkRequest(HTTPRequest& request) {
  std::string    protocol = request.getProtocol();
  std::string    method = request.getMethod();
  std::string    uri = request.getURI();
  unsigned int   contentLength = request.getContentLength();
  LocationConfig location = _getLocationConfig(uri);
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

  return NULL;
}

std::string VirtualServer::_generateDirectoryListing(const std::string& path) {
  DIR*           dir;
  struct dirent* entry;
  struct stat    statbuf;

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
      html << "<a href=\"" << URI::encode(entryName) << "\">" << entryName
           << "</a>";
      html << "</li>";
    }
    closedir(dir);
  } else {
    html << "<p>Error opening directory.</p>";
  }

  html << "</ul></body></html>";
  return html.str();
}

HTTPResponse* VirtualServer::_autoindex(const std::string& path,
                                        LocationConfig&    location) {
  std::string   indexPath = path + "/index.html";
  std::ifstream indexFile(indexPath.c_str());
  if (indexFile && location.autoindex) {
    std::string   content((std::istreambuf_iterator<char>(indexFile)),
                          std::istreambuf_iterator<char>());
    HTTPResponse* response = new HTTPResponse(200);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Length", Utils::to_string(content.size()));
    response->setBody(content);
    return response;
  } else if (location.autoindex) {
    // Générer une liste de fichiers et de répertoires
    std::string   directoryListing = _generateDirectoryListing(path);
    HTTPResponse* response = new HTTPResponse(200);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Length",
                        Utils::to_string(directoryListing.size()));
    response->setBody(directoryListing);
    return response;
  } else {
    _log.error(
        "VirtualServer::_handleGetRequest : Directory index not available");
    return new HTTPResponse(404, location.error_pages);
  }
}

std::string VirtualServer::_getPath(const std::string& uri,
                                    LocationConfig&    location) {
  if (location.root.empty()) {
    return _serverConfig.root + uri;
  }
  return location.root + uri;
}

HTTPResponse* VirtualServer::_handleGetRequest(HTTPRequest& request) {
  std::string uri = request.getURI();
  _log.info("VirtualServer::_handleGetRequest : URI : " + uri);
  LocationConfig location = _getLocationConfig(uri);
  std::string    path = _getPath(uri, location);
  struct stat    statbuf;
  if (stat(path.c_str(), &statbuf) == -1) {
    _log.error("VirtualServer::_handleGetRequest : Path not found: " + path);
    return new HTTPResponse(404, location.error_pages);
  }

  if (S_ISDIR(statbuf.st_mode)) {
    return _autoindex(path, location);
  }
  // Gestion des fichiers normaux
  std::string   contentType = HTTPResponse::getContentType(path);
  std::ifstream file(path.c_str());
  if (file) {
    std::string   content((std::istreambuf_iterator<char>(file)),
                          std::istreambuf_iterator<char>());
    HTTPResponse* response = new HTTPResponse(200);
    response->addHeader("Content-Type", contentType);
    response->addHeader("Content-Length", Utils::to_string(content.size()));
    response->setFile(path);
    return response;
  }

  _log.error("VirtualServer::_handleGetRequest : File not found");
  return new HTTPResponse(404, location.error_pages);
}

std::map<int, std::string> VirtualServer::_getErrorPages(
    const std::string& uri) {
  std::map<int, std::string> errorPages;
  LocationConfig             location = _getLocationConfig(uri);
  for (std::map<int, std::string>::iterator it = location.error_pages.begin();
       it != location.error_pages.end(); ++it) {
    errorPages[it->first] = it->second;
  }
  return errorPages;
}

HTTPResponse* VirtualServer::handleRequest(HTTPRequest& request) {
  std::string    protocol = request.getProtocol();
  std::string    method = request.getMethod();
  std::string    uri = request.getURI();
  LocationConfig location = _getLocationConfig(uri);
  request.setConfig(&location);
  _log.info("VirtualServer::handleRequest : Protocol : " + protocol +
            " Method : " + method + " URI : " + uri);

  if (protocol != "HTTP/1.1") {
    _log.error("VirtualServer::handleRequest : Protocol not supported");
    return new HTTPResponse(400, location.error_pages);
  }
  if (CGIHandler::isScript(request)) {
    _log.info("Handling CGI request for URI: " + request.getURI());
    try {
      return CGIHandler::processRequest(request);
    } catch (const CGIHandler::CGIDisabled& e) {
      return new HTTPResponse(HTTPResponse::FORBIDDEN, location.error_pages);
    } catch (const CGIHandler::TimeoutException& e) {
      return new HTTPResponse(HTTPResponse::GATEWAY_TIMEOUT,
                              location.error_pages);
    } catch (const Exception& e) {
      return new HTTPResponse(HTTPResponse::INTERNAL_SERVER_ERROR,
                              location.error_pages);
    }
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
  return new HTTPResponse(403, location.error_pages);
}

std::string VirtualServer::getServerName() const {
  return _serverConfig.server_names[0];
}

VirtualServer::~VirtualServer() {}
