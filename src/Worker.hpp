/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Worker.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 12:06:51 by mchenava          #+#    #+#             */
/*   Updated: 2024/05/23 12:14:00 by  mchenava        ###   ########.fr       */
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

#define MAX_EVENTS 10

class Worker {
	private:
		pthread_t		_thread;
		Config&			_config;
		Logger&			_log;
		int				_epollSocket;
		int				_maxConnections;
		int				_currentConnections;

		static void*	_workerRoutine(void *ref);

		int				_setupEpoll();
		void			_runEventLoop();
		void			_handleIncomingConnection(int fd);
		void			_handleOutgoingData(int fd);

	public:
		Worker(Config& config);
		~Worker();
		void	assignConnection(int clientSocket);

};

#endif
