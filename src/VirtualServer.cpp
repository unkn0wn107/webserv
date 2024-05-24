/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 15:10:25 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/24 17:02:27 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualServer.hpp"
#include "Utils.hpp"

VirtualServer::VirtualServer(ServerConfig& serverConfig) :
	_serverConfig(serverConfig),
	_log(Logger::getInstance())
{
	_hostNames = _serverConfig.server_names;
	_defaultServer = _hasDefaultListenConfig();
}

bool VirtualServer::_hasDefaultListenConfig() {
    for (std::vector<ListenConfig>::const_iterator it = _serverConfig.listen.begin(); it != _serverConfig.listen.end(); ++it) {
        if (it->default_server) {
            return true;  // Retourne vrai dès qu'un ListenConfig avec default_server à true est trouvé
        }
    }
    return false;  // Retourne faux si aucun ListenConfig avec default_server à true n'est trouvé
}

bool VirtualServer::isDefaultServer() {
	return _defaultServer;
}

bool VirtualServer::isHostMatching(const std::string& host) const {
    for (std::vector<std::string>::const_iterator it = _hostNames.begin(); it != _hostNames.end(); ++it) {
        if (*it == host) {
            return true;  // Retourne vrai dès qu'un nom d'hôte correspondant est trouvé
        }
    }
    return false;  // Retourne faux si aucun nom d'hôte correspondant n'est trouvé
}

void VirtualServer::parseRequest(const std::string& request, size_t readn) {
	_log.info("VirtualServer::parseRequest" + _serverConfig.server_names[0]);
	_log.info(request);
	_log.info(Utils::to_string<size_t>(readn));
}

std::string VirtualServer::getServerName() const {
	return _serverConfig.server_names[0];
}

VirtualServer::~VirtualServer() {
}
