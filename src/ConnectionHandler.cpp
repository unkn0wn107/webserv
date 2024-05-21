/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:21 by agaley            #+#    #+#             */
/*   Updated: 2024/05/07 09:05:51 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionHandler.hpp"

ConnectionHandler::ConnectionHandler(int socket, ServerConfig& config)
    : _socket(socket), _responseSent(0), _config(config) {}

ConnectionHandler::~ConnectionHandler() {
  close(_socket);
}

// HTTPProtocol* ConnectionHandler::selectHTTPProtocolVersion(
//     const std::string& requestString) {
//   std::istringstream iss(requestString);
//   std::string        method, url, version;
//   iss >> method >> url >> version;

//   if (version == "HTTP/1.1") {
//     return new HTTP1_1();
//   } else {
//     ErrorHandler::log("Unknown HTTP protocol version");
//   }
// }

void ConnectionHandler::process() {
  char    buffer[1024];
  ssize_t bytes_read = recv(_socket, buffer, sizeof(buffer), 0);
  if (bytes_read <= 0) {
    // Handle error or close connection
    return;
  }

  try {
    HTTP1_1 protocol(_config);
    _request = protocol.parseRequest(buffer);
  } catch (const std::exception& e) {
    _response.setStatusCode(HTTPResponse::BAD_REQUEST);
    _response.setBody("Error parsing request: " + std::string(e.what()));
    _responseBuffer = _response.generate();
    return;
  }

  std::string url = _request.getUrl();
  if (CGIHandler::isScript(url))
    _response = CGIHandler::processRequest(_request);
  else
    _response = FileHandler::processRequest(_request, _config);
  _responseBuffer = _response.generate();
}

bool ConnectionHandler::hasDataToSend() const {
  return _responseSent < _responseBuffer.size();
}

void ConnectionHandler::sendResponse() {
  if (_responseSent < _responseBuffer.size()) {
    ssize_t bytes_sent = send(_socket, _responseBuffer.c_str() + _responseSent,
                              _responseBuffer.size() - _responseSent, 0);
    if (bytes_sent > 0) {
      _responseSent += bytes_sent;
    }
    // Handle partial send or errors appropriately
  }
}
