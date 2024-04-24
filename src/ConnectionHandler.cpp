/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:30:33 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 17:36:05 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectionHandler.class.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>

ConnectionHandler::ConnectionHandler(int clientSocket, HTTPProtocol& protocol)
    : _log(Logger::getInstance()) , _clientSocket(clientSocket), _protocol(protocol) , _isCompleted(false) {}

ConnectionHandler::~ConnectionHandler() {
    close(_clientSocket);
}

void ConnectionHandler::handleRequest() {
    receiveData();
    if (!_request.isEmpty()) {  // Supposons que Request a une méthode isEmpty pour vérifier si des données ont été reçues
        _protocol.processRequest(_request, _response);  // Modifié pour utiliser les objets Request et Response
        sendData();
    }
    _isCompleted = true;
}

void ConnectionHandler::receiveData() {
    char buffer[1024];
    ssize_t bytesReceived = recv(_clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        _request.parse(buffer, bytesReceived);  // Supposons que Request a une méthode parse pour ajouter et traiter les données reçues
    } else if (bytesReceived == 0) {
        std::cout << "Client disconnected." << std::endl;
    } else {
        std::cerr << "Failed to receive data." << std::endl;
    }
}

void ConnectionHandler::sendData() {
    std::string data = _response.generate();  // Supposons que Response a une méthode generate pour créer la réponse HTTP complète
    ssize_t totalSent = 0;
    ssize_t bytesSent;
    while (totalSent < (ssize_t)data.size()) {
        bytesSent = send(_clientSocket, data.c_str() + totalSent, data.size() - totalSent, 0);
        if (bytesSent == -1) {
            std::cerr << "Failed to send data." << std::endl;
            break;
        }
        totalSent += bytesSent;
    }
}

bool ConnectionHandler::isCompleted() {
    return _isCompleted;
}

