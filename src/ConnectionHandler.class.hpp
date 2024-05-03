/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectionHandler.class.hpp                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:30:50 by mchenava          #+#    #+#             */
/*   Updated: 2024/05/03 12:52:00 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTIONHANDLER_HPP
#define CONNECTIONHANDLER_HPP

#include <string>
#include "HTTPProtocol.class.hpp"
#include "Request.class.hpp"
#include "Response.class.hpp"

class ConnectionHandler{
public:
    ConnectionHandler(int clientSocket, HTTPProtocol* protocol);
    ConnectionHandler(const ConnectionHandler& other);
    ConnectionHandler& operator=(const ConnectionHandler& other);
    ~ConnectionHandler();

    void handleRequest();
    void receiveData();
    void sendData();
    bool isCompleted();

private:
    Logger& _log;
    int _clientSocket;
    HTTPProtocol* _protocol; //Pointeur sur le protocole HTTP
    bool _isCompleted;

    Request* _request;
    Response* _response;
};

#endif // CONNECTIONHANDLER_HPP
