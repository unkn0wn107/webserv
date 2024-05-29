/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 10:22:15 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/29 16:13:39 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUAL_SERVER_HPP
#define VIRTUAL_SERVER_HPP

#include "Config.hpp"
#include "HTTPRequest.hpp"
#include "Logger.hpp"

class VirtualServer {
 private:
  ServerConfig&            _serverConfig;
  Logger&                  _log;
  bool                     _defaultServer;
  std::vector<std::string> _hostNames;

  bool            _hasDefaultListenConfig();
  LocationConfig& _getLocationConfig(const std::string& uri);

 public:
  VirtualServer(ServerConfig& serverConfig);
  ~VirtualServer();
  bool        isDefaultServer();
  bool        isHostMatching(const std::string& host) const;
  int         checkRequest(HTTPRequest& request);
  std::string getServerName() const;
};

#endif
