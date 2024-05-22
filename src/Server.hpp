/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:01 by agaley            #+#    #+#             */
/*   Updated: 2024/05/22 18:32:23 by mchenava         ###   ########.fr       */
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
		Config&							_config;
		Logger&							_log;
		std::vector<Worker*>			_workers;
		std::set<ListenConfig>			_uniqueConfigs;
		std::map<ListenConfig, int>		_listenSockets;

		void					_setupServerSockets(const std::vector<ServerConfig>& servers);

	public:
		Server(Config& config);
		~Server();
};

#endif
