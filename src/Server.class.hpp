/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.class.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:30:46 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 16:55:28 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <map>
#include "ConnectionHandler.class.hpp"
#include "Logger.class.hpp"

class Server {
public:
    Server(const std::string& configFilePath);
    Server(const Server& other);
    Server& operator=(const Server& other);
    ~Server();

    void run();

private:
    void _setupServerSockets();
    void _acceptConnections();
    void _processConnections();
    // Méthodes pour la gestion de la configuration
    void _loadConfiguration();
    void _createListeningSocket(int port);
    // Utilitaires
    void _setNonBlocking(int socketFd);
    HTTPProtocol& _getHTTPProtocol();

    Logger& _log;
    HTTPProtocol* _httpProtocol;
    std::string _configFilePath;
    std::vector<int> _listeningSockets;
    std::map<int, ConnectionHandler*> _connectionHandlers; // Associe les descripteurs de fichier aux gestionnaires de connexion

};

#endif // SERVER_HPP
