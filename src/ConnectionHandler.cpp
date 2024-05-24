/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:21 by agaley            #+#    #+#             */
/*   Updated: 2024/05/24 17:04:51 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionHandler.hpp"
#include <string>
#include <sstream>
#include <algorithm>

ConnectionHandler::ConnectionHandler(int clientSocket, int epollSocket, ListenConfig& listenConfig, std::vector<VirtualServer*>& virtualServers):
	_log(Logger::getInstance()),
	_listenConfig(listenConfig),
	_connectionStatus(READING),
	_clientSocket(clientSocket),
	_epollSocket(epollSocket),
	_readn(0),
	_vservPool(virtualServers)
{
	_buffer = new char[1024]; //TODO: check if it's ok
	memset(_buffer, 0, 1024);
}

ConnectionHandler::~ConnectionHandler() {
	delete[] _buffer;
}

void ConnectionHandler::_receiveRequest() {
	bool   end = false;
	while (true) {
		ssize_t	bytes = recv(_clientSocket, _buffer + _readn, 1, 0);
		if (_readn >= 1024) { //adapt to buffer size
			_connectionStatus = CLOSED;
			return;
		}
		if (bytes < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
					break;
			} else {
					_log.error(std::string("CONNECTION_HANDLER: recv: ") + strerror(errno));
					break;
			}
		}
		else if (bytes == 0) {
			break;
		}
		else {
			_readn++;
			if (_readn >= 4 
				&& _buffer[_readn - 1] == '\n' 
				&& _buffer[_readn - 2] == '\r'
				&& _buffer[_readn - 3] == '\n'
				&& _buffer[_readn - 4] == '\r') {
					end = true;
					break;
			}
		}
	}
	if (!end) return;
	_processRequest();
}

VirtualServer* ConnectionHandler::_selectVirtualServer() {
    std::string requestHeader(_buffer, _readn);  // Convertir le buffer en string pour faciliter la manipulation
    std::string host = _extractHost(requestHeader);  // Méthode hypothétique pour extraire le champ Host

    if (host.empty()) {
        return _findDefaultServer();  // Trouver le serveur par défaut si aucun Host n'est spécifié
    }

    // Parcourir la liste des serveurs virtuels pour trouver une correspondance
    for (std::vector<VirtualServer*>::iterator it = _vservPool.begin(); it != _vservPool.end(); ++it) {
        if ((*it)->isHostMatching(host)) {  // Supposons que VirtualServer a une méthode pour vérifier le nom d'hôte
            return *it;
        }
    }

    // Si aucun serveur correspondant n'est trouvé, retourner le serveur par défaut
    return _findDefaultServer();
}

VirtualServer* ConnectionHandler::_findDefaultServer() {
    for (std::vector<VirtualServer*>::iterator it = _vservPool.begin(); it != _vservPool.end(); ++it) {
        if ((*it)->isDefaultServer()) {
            return *it;
        }
    }
    return NULL;
}

std::string ConnectionHandler::_extractHost(const std::string& requestHeader) {
    std::istringstream stream(requestHeader);
    std::string line;
    std::string hostPrefix = "Host: ";

    while (std::getline(stream, line)) {
        // Normaliser la ligne pour la comparaison
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        // Vérifier si la ligne commence par "host:"
        size_t pos = line.find(hostPrefix);
        if (pos != std::string::npos) {
            // Extraire la valeur de l'hôte après "Host: "
            std::string hostValue = line.substr(pos + hostPrefix.length());

            // Supprimer les espaces de début et de fin
            size_t first = hostValue.find_first_not_of(" \t");
            size_t last = hostValue.find_last_not_of(" \r\n");
            if (first != std::string::npos && last != std::string::npos) {
                return hostValue.substr(first, (last - first + 1));
            }
            break;
        }
    }

    return "";  // Retourner une chaîne vide si aucun champ Host n'est trouvé
}

void ConnectionHandler::_processRequest() {
	VirtualServer*	vserv = _selectVirtualServer();
	if (vserv == NULL) {
		_log.error("CONNECTION_HANDLER: No virtual server selected");
		return;
	}
	_log.info(std::string("CONNECTION_HANDLER: Selected virtual server: ") + vserv->getServerName());
	vserv->parseRequest(_buffer, _readn);
}

void ConnectionHandler::processConnection() {
	// struct epoll_event	event;
	// event.data.fd = _clientSocket;
	// event.events = EPOLLIN;
	if (_connectionStatus == READING) {
		_receiveRequest();
	}
	else if (_connectionStatus == SENDING) {
		// sendResponse();
	}
	else if (_connectionStatus == CLOSED) {
		close(_clientSocket);
	}
}
