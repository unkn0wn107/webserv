/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPMethods.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 11:59:07 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/25 19:36:03 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPMethods.hpp"

HTTPMethods::HTTPMethods(VirtualServer& server):
	_server(server),
	_log(Logger::getInstance())
{}

HTTPMethods::~HTTPMethods() {

}

std::string HTTPMethods::_getPath(const std::string& uri,
                                    LocationConfig&    location) {
  if (location.root.empty()) {
    return _server.getRoot() + "/" + uri;
  }
  return location.root + "/" + uri;
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
                                      LocationConfig&    location) {
  std::string indexPath = path + location.index;
  if (FileManager::doesFileExists(indexPath)) {
    HTTPResponse* response = new HTTPResponse(HTTPResponse::OK);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Length", Utils::to_string(FileManager::getFileSize(indexPath)));
    response->setFile(indexPath);
    return response;
  } else if (location.autoindex) {
    std::string   directoryListing = _generateDirectoryListing(path);
    HTTPResponse* response = new HTTPResponse(HTTPResponse::OK);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Length",
                        Utils::to_string(directoryListing.size()));
    response->setBody(directoryListing);
    return response;
  } else {
    _log.error(
        "HTTPMethods::_autoindex : Directory index not available");
    return new HTTPResponse(HTTPResponse::NOT_FOUND, location.error_pages);
  }
}

HTTPResponse* HTTPMethods::_handleGetRequest(HTTPRequest& request) {
  std::string uri = request.getURI();
  LocationConfig location = _server.getLocationConfig(uri);
  _log.info("HTTPMethods::_handleGetRequest : scriptName: " + request.getURIComponents().scriptName);
  std::string    path = _getPath(request.getURIComponents().scriptName, location);
  _log.info("HTTPMethods::_handleGetRequest : path: " + path);
  struct stat    statbuf;
  if (stat(path.c_str(), &statbuf) == -1) {
    _log.error("HTTPMethods::_handleGetRequest : Path not found: " + path);
    return new HTTPResponse(HTTPResponse::NOT_FOUND, location.error_pages);
  }

  if (FileManager::isDirectory(path)) {
    return _autoindex(path, location);
  }
  std::string   contentType = HTTPResponse::getContentType(path);
  if (FileManager::doesFileExists(path)) {
    HTTPResponse* response = new HTTPResponse(HTTPResponse::OK);
    response->addHeader("Content-Type", contentType);
    response->addHeader("Content-Length", Utils::to_string(FileManager::getFileSize(path)));
    response->setFile(path);
    return response;
  }

  _log.error("HTTPMethods::_handleGetRequest : File not found");
  return new HTTPResponse(HTTPResponse::NOT_FOUND, location.error_pages);
}

HTTPResponse* HTTPMethods::_handlePostRequest(HTTPRequest& request) {
	LocationConfig location = _server.getLocationConfig(request.getURI());
	if (location.upload == false) {
		return new HTTPResponse(HTTPResponse::FORBIDDEN, location.error_pages);
	}
	std::string path = _getPath(request.getURI(), location);
	std::string contentType;
  contentType = request.getHeader("Content-Type");
  if (contentType == "") {
    contentType = HTTPResponse::getContentType(path);
  }
  else {
    std::string extension = HTTPResponse::getExtensionFromContentType(contentType);
    path += "." + extension;
  }
	std::ofstream file(path.c_str());
	if (file) {
		if (!file.write(request.getBody().c_str(), request.getBody().size())) {
      _log.error("HTTPMethods::_handlePostRequest : File write error: " + path);
			return new HTTPResponse(HTTPResponse::INTERNAL_SERVER_ERROR, location.error_pages);
		}
		file.close();
		HTTPResponse* response = new HTTPResponse(HTTPResponse::OK);
		response->addHeader("Content-Type", "text/html");
		response->addHeader("Content-Length", Utils::to_string(path.size() + 67));
		response->setBody("<html><body>File uploaded successfully. File path: " + path + "</body></html>");
		return response;
	}
	_log.error("HTTPMethods::_handlePostRequest : File open error: " + path + " | error: " + strerror(errno));
	return new HTTPResponse(HTTPResponse::INTERNAL_SERVER_ERROR, location.error_pages);
}

HTTPResponse* HTTPMethods::_handleDeleteRequest(HTTPRequest& request) {
	LocationConfig location = _server.getLocationConfig(request.getURI());
	if (location.delete_ == false) {
		return new HTTPResponse(HTTPResponse::FORBIDDEN, location.error_pages);
	}
	std::string path = _getPath(request.getURI(), location);
	if (remove(path.c_str()) == 0) {
    HTTPResponse* response = new HTTPResponse(HTTPResponse::OK);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Length", Utils::to_string(path.size() + 66));
    response->setBody("<html><body>File deleted successfully. File path: " + path + "</body></html>");
		return response;
	} else {
    _log.error("HTTPMethods::_handleDeleteRequest : File delete error");
		return new HTTPResponse(HTTPResponse::INTERNAL_SERVER_ERROR, location.error_pages);
	}
}

HTTPResponse* HTTPMethods::_handleHeadRequest(HTTPRequest& request) {
	HTTPResponse* response = _handleGetRequest(request);
	response->setBody("");
  response->setFile("");
  response->deleteHeader("Content-Length");
	return response;
}

HTTPResponse* HTTPMethods::handleRequest(HTTPRequest& request) {
	std::string    protocol = request.getProtocol();
  std::string    method = request.getMethod();
  std::string    uri = request.getURI();
  LocationConfig location = _server.getLocationConfig(uri);
  request.setConfig(&location);

  if (protocol != "HTTP/1.1") {
    _log.error("HTTPMethods::handleRequest : Protocol not supported");
    return new HTTPResponse(HTTPResponse::BAD_REQUEST, location.error_pages);
  }
  if (location.cgi && CGIHandler::isScript(request)) {
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
  if (method == "POST") {
    return _handlePostRequest(request);
  }
  if (method == "DELETE") {
    return _handleDeleteRequest(request);
  }
  if (method == "HEAD") {
    return _handleHeadRequest(request);
  }
  _log.error("HTTPMethods::handleRequest : Method not allowed");
  return new HTTPResponse(HTTPResponse::NOT_IMPLEMENTED, location.error_pages);
}
