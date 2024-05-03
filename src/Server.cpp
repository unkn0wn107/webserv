/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:33:25 by mchenava          #+#    #+#             */
/*   Updated: 2024/05/03 12:54:24 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>  // Pour memset
#include <iostream>
#include <sstream>
#include "HTTP1_1.class.hpp"
#include "Server.class.hpp"
#include "ServerException.class.hpp"

Server::Server(const std::string& configFilePath)
    : _log(Logger::getInstance()), _configFilePath(configFilePath) {
  _loadConfiguration();
  _setupServerSockets();
  _log.info("Server initialized");
}

Server::~Server() {
  for (std::vector<int>::iterator it = _listeningSockets.begin();
      it != _listeningSockets.end(); ++it) {
    int socketFd = *it;
    close(socketFd);
  }
  for (std::map<int, ConnectionHandler*>::iterator it = 
    _connectionHandlers.begin();
    it != _connectionHandlers.end(); ++it) {
      delete it->second;
  }
}

void Server::run() {
  while (true) {
    _acceptConnections();
    _processConnections();
    // Ajouter ici la logique pour gérer les requêtes et les réponses
  }
}

void Server::_setupServerSockets() {
  // Exemple pour un seul port. Étendre selon la configuration.
  int port = 8080;  // Port par défaut, à remplacer par la valeur lue dans le
                    // fichier de configuration
  _createListeningSocket(port);
}

void Server::_createListeningSocket(int port) {
  int serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd == -1) {
    throw ServerException("Failed to create socket");
  }

  int opt = 1;
  if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    throw ServerException("Failed to set socket options");
  }

  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) == -1) {
    throw ServerException("Failed to bind");
  }

  if (listen(serverFd, 10) == -1) {  // Backlog de 10 connexions en attente
    throw ServerException("Failed to listen");
  }

  _setNonBlocking(serverFd);
  _listeningSockets.push_back(serverFd);
  std::ostringstream str_port;
  str_port << "Listening on port " << port;
  _log.info(str_port.str());
}

void Server::_acceptConnections() {
  for (std::vector<int>::iterator it = _listeningSockets.begin();
       it != _listeningSockets.end(); ++it) {
    int listeningSocket = *it;
    int clientSocket = accept(listeningSocket, NULL, NULL);
    if (clientSocket == -1) {
      continue;  // Pas de connexion entrante sur ce tour de boucle
    }
    _setNonBlocking(clientSocket);
    _connectionHandlers[clientSocket] = new ConnectionHandler(
        clientSocket, new HTTP1_1());
    _log.info("New connection accepted");
  }
}

void Server::_processConnections() {
  // Ici, vous pouvez itérer sur connectionHandlers et appeler handleRequest
  // pour chaque handler
  // _log.info("Processing connections");
  for (std::map<int, ConnectionHandler*>::iterator it =
           _connectionHandlers.begin();
       it != _connectionHandlers.end(); ++it) {
    ConnectionHandler* handler = it->second;
    handler->handleRequest();
    if (handler->isCompleted()) {
      _log.info("Connection completed");
      delete handler;
      _connectionHandlers.erase(it->first);
    }
  }
}

void Server::_setNonBlocking(int socketFd) {
  int flags = fcntl(socketFd, F_GETFL, 0);
  if (flags == -1) {
    throw ServerException("Failed to get socket flags");
  }
  if (fcntl(socketFd, F_SETFL, flags | O_NONBLOCK) == -1) {
    throw ServerException("Failed to set socket non-blocking");
  }
}

void Server::_loadConfiguration() {
  // Implémentez la logique de chargement de la configuration ici
  // Cela pourrait impliquer la lecture du fichier de configuration et
  // l'initialisation des variables de configuration
  _log.warning("NO CONFIGURATION FILE");
}

HTTPProtocol& Server::_getHTTPProtocol() {
  if (_httpProtocol == NULL) {
    _httpProtocol = new HTTP1_1();
  }
  return *_httpProtocol;
}

