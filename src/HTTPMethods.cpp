/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPMethods.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 11:59:07 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/11 12:46:33 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPMethods.hpp"

HTTPMethods::HTTPMethods(VirtualServer& server): 
	_server(server),
	_log(Logger::getInstance()) 
{}

HTTPMethods::~HTTPMethods() {}

std::string HTTPMethods::_getPath(const std::string& uri,
                                    LocationConfig&    location) {
  if (location.root.empty()) {
    return _server.getRoot() + uri;
  }
  return location.root + uri;
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
  std::string   indexPath = path + "/" + location.index;
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
    std::string   directoryListing = _generateDirectoryListing(path);
    HTTPResponse* response = new HTTPResponse(200);
    response->addHeader("Content-Type", "text/html");
    response->addHeader("Content-Length",
                        Utils::to_string(directoryListing.size()));
    response->setBody(directoryListing);
    return response;
  } else {
    _log.error(
        "HTTPMethods::_autoindex : Directory index not available");
    return new HTTPResponse(404, location.error_pages);
  }
}

HTTPResponse* HTTPMethods::_handleGetRequest(HTTPRequest& request) {
  std::string uri = request.getURI();
  _log.info("HTTPMethods::_handleGetRequest : URI : " + uri);
  LocationConfig location = _server.getLocationConfig(uri);
  std::string    path = _getPath(uri, location);
  struct stat    statbuf;
  if (stat(path.c_str(), &statbuf) == -1) {
    _log.error("HTTPMethods::_handleGetRequest : Path not found: " + path);
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

  _log.error("HTTPMethods::_handleGetRequest : File not found");
  return new HTTPResponse(404, location.error_pages);
}

HTTPResponse* HTTPMethods::_handlePostRequest(HTTPRequest& request) {
	LocationConfig location = _server.getLocationConfig(request.getURI());
	if (location.upload == false) {
		return new HTTPResponse(403, location.error_pages);
	}
	std::string path = _getPath(request.getURI(), location);
	std::string contentType = request.getHeader("Content-Type");
	std::string extension = HTTPResponse::getExtensionFromContentType(contentType);
	std::string filePath = path + "." + extension;
	std::ofstream file(filePath.c_str());
	if (file) {
		if (!file.write(request.getBody().c_str(), request.getBody().size())) {
			return new HTTPResponse(500, location.error_pages);
		}
		file.close();
		HTTPResponse* response = new HTTPResponse(200);
		response->addHeader("Content-Type", "text/html");
		response->addHeader("Content-Length", Utils::to_string(filePath.size() + 67));
		response->setBody("<html><body>File uploaded successfully. File path: " + filePath + "</body></html>");
		return response;
	}
	return new HTTPResponse(500, location.error_pages);
}

HTTPResponse* HTTPMethods::handleRequest(HTTPRequest& request) {
	std::string    protocol = request.getProtocol();
  std::string    method = request.getMethod();
  std::string    uri = request.getURI();
  LocationConfig location = _server.getLocationConfig(uri);
  request.setConfig(&location);
  _log.info("HTTPMethods::handleRequest : Protocol : " + protocol +
            " Method : " + method + " URI : " + uri);

  if (protocol != "HTTP/1.1") {
    _log.error("HTTPMethods::handleRequest : Protocol not supported");
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
  if (method == "POST") {
    return _handlePostRequest(request);
  }
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
  _log.error("HTTPMethods::handleRequest : Method not allowed");
  return new HTTPResponse(403, location.error_pages);
}
