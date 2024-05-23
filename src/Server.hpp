/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/05/23 12:08:12 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include "Config.hpp"
#include "Logger.hpp"
#include "Worker.hpp"
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

class ConnectionHandler;

class Server {
	private:
		Config&												_config;
		Logger&												_log;
		std::vector<Worker*>					_workers;
		int														_workerIndex;
		std::map<ListenConfig, int>		_listenSockets;

		void					_setupServerSockets();
		void					_setupWorkers();
	public:
		Server(Config& config);
		~Server();
};

#endif
