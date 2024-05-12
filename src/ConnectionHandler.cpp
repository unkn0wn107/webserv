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

ConnectionHandler::ConnectionHandler(std::vector<ServerConfig>& configs): _responseSent(0), _configs(configs) {
  for (ServerConfig it = _configs.begin(); it != _configs.end(); ++it) {
    _servers.push_back(Server(*it));
  }
}

ConnectionHandler::~ConnectionHandler() {

}

void ConnectionHandler::handleConnection(int socket) {
    _servers.push_back(Server(config, socket));
}

void ConnectionHandler::_loadServers() {
  for (ServerConfig& config : _configs) {
    _servers.push_back(Server(config, so));
  }
}
