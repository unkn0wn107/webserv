/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 12:06:51 by mchenava          #+#    #+#             */
/*   Updated: 2024/05/28 08:39:28 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WORKER_HPP
#define WORKER_HPP

#include <pthread.h>
#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/socket.h>
#include "Logger.hpp"
#include "Utils.hpp"
#include "ConnectionHandler.hpp"
#include "VirtualServer.hpp"

#define MAX_EVENTS 10

class ConnectionHandler;

class Worker {
	private:
		pthread_t										_thread;
		Config&											_config;
		Logger&											_log;
		std::map<int, ConnectionHandler*>				_handlers;
		std::map<int, ListenConfig>						_listenConfigs;
		std::map<int, std::vector<VirtualServer*> >		_virtualServers;
		int												_epollSocket;
		std::vector<int>								_listenSockets;
		int												_maxConnections;
		int												_currentConnections;
		pthread_mutex_t									*_epollMutex;

		static void*	_workerRoutine(void *ref);

		void				_setupEpoll();
		std::vector<VirtualServer*>	_setupAssociateVirtualServer(const ListenConfig& listenConfig);
		void			_acceptNewConnection(int fd);
		void			_runEventLoop();
		void			_handleIncomingConnection(struct epoll_event& event);

	public:
		Worker(pthread_mutex_t* epollMutex);
		~Worker();
		void	assignConnection(int clientSocket, const ListenConfig& listenConfig);

};

#endif
