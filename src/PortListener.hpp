/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PortListener.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 17:31:07 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/11 18:33:15 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PORTLISTENER_HPP
#define PORTLISTENER_HPP

#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <thread>
#include "ConnectionHandler.hpp"
#include "ConfigLoader.hpp"
#include "Config.hpp"

class PortListener {
public:
    PortListener(int port);
    ~PortListener();

    void startListening();
    void stopListening();

private:
    int _port;
    int _epoll_fd;
    bool _keepRunning;
		int	_listeningSocket;
  	std::set<int>                      _client_sockets;
		std::vector<ServerConfig> _serverConfigs;
    std::thread _listeningThread;
		ConnectionHandler _connectionHandler;

    void _listenThread();
    void _acceptConnection();
    void _setupEpoll();
		void _setupListeningSocket();
		void _setupEpoll();
};

#endif
