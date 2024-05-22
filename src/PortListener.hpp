/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PortListener.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/11 17:31:07 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/21 15:38:19 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PORTLISTENER_HPP
#define PORTLISTENER_HPP

#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <pthread.h>
#include "ConnectionHandler.hpp"
#include "Config.hpp"
#include "Logger.hpp"

class PortListener {
public:
    PortListener(int port, std::vector<ServerConfig> configs);
    ~PortListener();

    void startListening();
    void stopListening();
    static void* listenThreadStatic(void* arg);

private:
    int _port;
    int _server_fd;
    int _epoll_fd;
    bool _keepRunning;
		int	_listeningSocket;
  	std::set<int>                       _client_sockets;
		std::vector<ServerConfig>           _serverConfigs;
    pthread_t                           _listeningThread;
		ConnectionHandler* _connectionHandler;
    Logger&                              _log;


    void _listenThread();
    void _acceptConnection();
    void _setupEpoll();
		void _setupListeningSocket();
};

#endif
