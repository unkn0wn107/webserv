/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 10:22:15 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/24 17:02:22 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUAL_SERVER_HPP
#define VIRTUAL_SERVER_HPP

#include "Config.hpp"
#include "Logger.hpp"

class	VirtualServer {
	private:
		ServerConfig&			_serverConfig;
		Logger&						_log;
		bool							_defaultServer;
		std::vector<std::string>	_hostNames;

		bool							_hasDefaultListenConfig();


	public:
		VirtualServer(ServerConfig& serverConfig);
		~VirtualServer();
		bool							isDefaultServer();
		bool							isHostMatching(const std::string& host) const;
		void							parseRequest(const std::string& request, size_t readn);
		std::string				getServerName() const;
};

#endif
