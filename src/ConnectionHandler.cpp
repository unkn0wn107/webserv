/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:21 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 17:11:31 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionHandler.hpp"

ConnectionHandler::ConnectionHandler(Server& server, int socket)
    : _server(server), _socket(socket) {}

ConnectionHandler::~ConnectionHandler() {
  close(_socket);
}

HTTPProtocol* ConnectionHandler::selectHTTPProtocolVersion(
    const std::string& requestString) {
  std::istringstream iss(requestString);
  std::string        method, url, version;
  iss >> method >> url >> version;

  if (version == "HTTP/1.1") {
    return new HTTP1_1();
  } else {
    ErrorHandler::log("Unknown HTTP protocol version");
  }
}

void ConnectionHandler::process() {
  char    buffer[1024];
  ssize_t bytes_read = recv(_socket, buffer, sizeof(buffer), 0);
  if (bytes_read <= 0) {
    // Handle error or close connection
    return;
  }

  try {
    _request.parse(buffer);
  } catch (const std::exception& e) {
    _response.setStatusCode(HTTPResponse::BAD_REQUEST);
    _response.setBody("Error parsing request: " + std::string(e.what()));
    sendResponse();
    return;
  }

  std::string url = _request.getUrl();
  std::string extension = url.substr(url.find_last_of('.'));

  if (CGIHandler::isScript(url))
    _response = CGIHandler::processRequest(_request);
  else
    _response = FileHandler::processRequest(_request);
  sendResponse();
}

void ConnectionHandler::sendResponse() {
  std::string response = _response.generate();
  send(_socket, response.c_str(), response.size(), 0);
}
