/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:02 by agaley            #+#    #+#             */
/*   Updated: 2024/05/07 09:16:32 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Config.hpp"
#include "Logger.hpp"

Server::Server(ServerConfig& config) :
  _config(config),
  _log(Logger::getInstance())
{
}

Server::~Server() {
}

bool Server::isForMe(std::string host)
{
  return std::find(_config.server_names.begin(), _config.server_names.end(), host) != _config.server_names.end();
}

HTTPProtocol* Server::_getProtocol(char* buffer) {
    std::string request(buffer);
    std::string protocolIndicator = "HTTP/";

    size_t protocolPos = request.find(protocolIndicator);
    if (protocolPos == std::string::npos) {
        return NULL;
    }
    size_t versionStart = protocolPos + protocolIndicator.length();
    size_t versionEnd = request.find("\r\n", versionStart); // La version du protocole est suivie par un retour à la ligne
    if (versionEnd == std::string::npos) {
        return NULL;
    }
    std::string protocolVersion = request.substr(versionStart, versionEnd - versionStart);
    // if (protocolVersion == "1.0") {
    //     return new HTTP1_0(_config);
    // } else
    if (protocolVersion == "1.1") {
        return new HTTP1_1(_config);
    // } else if (protocolVersion == "2.0") {
    //     return new HTTP2_0(_config);
    } else {
        return NULL;
    }
}

void Server::processConnection(char *buffer) {
  try {
    HTTPProtocol* protocol = _getProtocol(buffer);
    _request = protocol->parseRequest(buffer);
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

// bool ConnectionHandler::hasDataToSend() const {
//   return _responseSent < _responseBuffer.size();
// }

// void ConnectionHandler::sendResponse() {
//   if (_responseSent < _responseBuffer.size()) {
//     ssize_t bytes_sent = send(_socket, _responseBuffer.c_str() + _responseSent,
//                               _responseBuffer.size() - _responseSent, 0);
//     if (bytes_sent > 0) {
//       _responseSent += bytes_sent;
//     }
//     // Handle partial send or errors appropriately
//   }
// }
