/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPMethods.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 11:59:07 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/27 15:19:16 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPMethods.hpp"

HTTPMethods::HTTPMethods(VirtualServer& server)
    : _server(server), _log(Logger::getInstance()) {}

HTTPMethods::~HTTPMethods() {}

std::string HTTPMethods::_getPath(const std::string& uriPath,
                                  const LocationConfig& location) {
  if (location.root.empty()) {
    return _server.getRoot() + uriPath;
  }
  return location.root + uriPath;
}

std::string HTTPMethods::_generateDirectoryListing(const std::string& path) {
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

HTTPResponse* HTTPMethods::_autoindex(const std::string& path,
                                      const LocationConfig& location) {
  std::string indexPath = path + location.index;
  if (FileManager::doesFileExists(indexPath)) {
    HTTPResponse* response = new HTTPResponse(HTTPResponse::OK, location);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Length",
                        Utils::to_string(FileManager::getFileSize(indexPath)));
    response->setFile(indexPath);
    return response;
  } else if (location.autoindex) {
    std::string   directoryListing = _generateDirectoryListing(path);
    HTTPResponse* response = new HTTPResponse(HTTPResponse::OK, location);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Length",
                        Utils::to_string(directoryListing.size()));
    response->setBody(directoryListing);
    return response;
  } else {
    _log.error("HTTPMethods::_autoindex : Directory index not available");
    return new HTTPResponse(HTTPResponse::NOT_FOUND, location);
  }
}

HTTPResponse* HTTPMethods::_handleGetRequest(HTTPRequest& request) {
  std::string    uriPath = request.getURIComponents().path;
  const LocationConfig& location = _server.getLocationConfig(uriPath);
  _log.info("HTTPMethods::_handleGetRequest : URI path: " + uriPath);
  std::string path = _getPath(uriPath, location);
  _log.info("HTTPMethods::_handleGetRequest : path: " + path);
  HTTPResponse* response;
  if (FileManager::isDirectory(path)) {
    response = _autoindex(path, location);
  } else if (FileManager::doesFileExists(path)) {
    std::string contentType = HTTPResponse::getContentType(path);
    response = new HTTPResponse(HTTPResponse::OK, location);
    response->addHeader("Content-Type", contentType);
    response->addHeader("Content-Length",
                        Utils::to_string(FileManager::getFileSize(path)));
    response->setFile(path);
  } else {
    _log.warning("HTTPMethods::_handleGetRequest : File not found");
    return new HTTPResponse(HTTPResponse::NOT_FOUND, location);
  }

  if (request.getHeader("Cache-Control") == "no-cache") {
    response->addHeader("Cache-Control", "no-cache");
  }
  return response;
}

HTTPResponse* HTTPMethods::_handlePostRequest(HTTPRequest& request) {
  std::string    uriPath = request.getURIComponents().path;
  const LocationConfig& location = _server.getLocationConfig(uriPath);
  if (location.upload == false) {
    return new HTTPResponse(HTTPResponse::FORBIDDEN, location);
  }
  std::string path = _getPath(uriPath, location);
  std::string contentType;
  contentType = request.getHeader("Content-Type");
  if (contentType == "") {
    contentType = HTTPResponse::getContentType(path);
    if (contentType == "") {
      _log.error("HTTPMethods::_handlePostRequest : No content type found");
      return new HTTPResponse(HTTPResponse::BAD_REQUEST, location);
    }
  } else {
    std::string extension = "";
    std::string is_extension = HTTPResponse::getContentType(path);
    if (is_extension == "") {
      extension = HTTPResponse::getExtensionFromContentType(contentType);
      path += "." + extension;
    }
  }
  std::ofstream file(path.c_str());
  if (file) {
    if (!file.write(request.getBody().c_str(), request.getBody().size())) {
      _log.error("HTTPMethods::_handlePostRequest : File write error: " + path);
      return new HTTPResponse(HTTPResponse::INTERNAL_SERVER_ERROR, location);
    }
    file.close();
    HTTPResponse* response = new HTTPResponse(HTTPResponse::CREATED, location);
    response->setHeaders(request.getHeaders());
    response->addHeader("Location", uriPath);
    response->setFile(path);
    return response;
  }
  _log.error("HTTPMethods::_handlePostRequest : File open error: " + path +
             " | error: " + strerror(errno));
  return new HTTPResponse(HTTPResponse::INTERNAL_SERVER_ERROR, location);
}

HTTPResponse* HTTPMethods::_handleDeleteRequest(HTTPRequest& request) {
  std::string    uriPath = request.getURIComponents().path;
  const LocationConfig& location = _server.getLocationConfig(uriPath);
  if (location.delete_ == false) {
    return new HTTPResponse(HTTPResponse::FORBIDDEN, location);
  }
  std::string path = _getPath(uriPath, location);
  if (remove(path.c_str()) == 0) {
    HTTPResponse* response = new HTTPResponse(HTTPResponse::OK, location);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Length", Utils::to_string(path.size() + 66));
    response->setBody("<html><body>File deleted.</body></html>");
    return response;
  } else {
    if (errno == ENOENT) {
      _log.warning("HTTPMethods::_handleDeleteRequest : File not found");
      return new HTTPResponse(HTTPResponse::NO_CONTENT, location);
    }
    _log.error("HTTPMethods::_handleDeleteRequest : File delete error");
    return new HTTPResponse(HTTPResponse::INTERNAL_SERVER_ERROR, location);
  }
}

HTTPResponse* HTTPMethods::handleRequest(HTTPRequest& request) {
  std::string    method = request.getMethod();
  LocationConfig location =
      _server.getLocationConfig(request.getURIComponents().path);
  if (request.getProtocol() != "HTTP/1.1") {
    _log.error("HTTPMethods::handleRequest : Protocol not supported");
    return new HTTPResponse(HTTPResponse::BAD_REQUEST, location);
  }

  HTTPResponse* response;
  if (method == "GET" || method == "HEAD") {
    response = _handleGetRequest(request);
  }
  if (method == "POST") {
    response = _handlePostRequest(request);
  }
  if (method == "DELETE") {
    response = _handleDeleteRequest(request);
  }
  if (method == "HEAD") {
    response->setBody("");
    response->setFile("");
    response->deleteHeader("Content-Length");
  }

  if (response)
    return response;

  _log.error("HTTPMethods::handleRequest : Method not allowed");
  return new HTTPResponse(HTTPResponse::NOT_IMPLEMENTED, location);
}
