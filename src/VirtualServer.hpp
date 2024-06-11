/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 10:22:15 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/11 12:46:05 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUAL_SERVER_HPP
#define VIRTUAL_SERVER_HPP

#include "Config.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "CGIHandler.hpp"
#include "HTTPMethods.hpp"


class HTTPMethods;

class VirtualServer {
 private:
  ServerConfig&            _serverConfig;
  Logger&                  _log;
  bool                     _defaultServer;
  std::vector<std::string>    _hostNames;
  HTTPMethods*                _httpMethods;

  bool            _hasDefaultListenConfig();
  std::map<int, std::string> _getErrorPages(const std::string& uri);
  

 public:
  VirtualServer(ServerConfig& serverConfig);
  ~VirtualServer();
  bool        isDefaultServer();
  bool        isHostMatching(const std::string& host) const;
  HTTPResponse *checkRequest(HTTPRequest& request);
  std::string getServerName() const;
  HTTPResponse *handleRequest(HTTPRequest& request);
  LocationConfig& getLocationConfig(const std::string& uri);
  std::string getRoot() const;
};

#endif
