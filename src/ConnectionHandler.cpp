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

ConnectionHandler::ConnectionHandler(std::vector<ServerConfig>& configs): _configs(configs) {
  _defaultServer = NULL;
  for (std::vector<ServerConfig>::iterator it = _configs.begin(); it != _configs.end(); ++it) {
    _servers.push_back(new Server(*it));
    if (it->isDefault)
      _defaultServer = _servers.back();
  }
}

ConnectionHandler::~ConnectionHandler() {

}

void ConnectionHandler::handleConnection(int socket) {
  char    buffer[1024];
  ssize_t bytes_read = recv(socket, buffer, sizeof(buffer), 0);
  if (bytes_read <= 0) {
    // Handle error or close connection
    return;
  }
  _redirect(buffer);
}

void ConnectionHandler::_redirect(char* buffer) {
    std::string request(buffer);
    std::string hostHeader = "Host: ";
    size_t hostPos = request.find(hostHeader);
    if (hostPos == std::string::npos) {
        return;
    }
    size_t hostStart = hostPos + hostHeader.length();
    size_t hostEnd = request.find("\r\n", hostStart);
    std::string host = request.substr(hostStart, hostEnd - hostStart);
    for (std::vector<Server *>::iterator server = _servers.begin(); server != _servers.end(); ++server){
      if ((*server)->isForMe(host)) return (*server)->processConnection(buffer);
    }
    _defaultServer->processConnection(buffer);
}
