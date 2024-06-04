/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 10:22:15 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/04 16:03:22 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUAL_SERVER_HPP
#define VIRTUAL_SERVER_HPP

#include "Config.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"
#include "CGIHandler.hpp"

class VirtualServer {
 private:
  ServerConfig&            _serverConfig;
  Logger&                  _log;
  bool                     _defaultServer;
  std::vector<std::string>    _hostNames;

  bool            _hasDefaultListenConfig();
  LocationConfig& _getLocationConfig(const std::string& uri);
  HTTPResponse *_handleGetRequest(HTTPRequest& request);
  std::map<int, std::string> _getErrorPages(const std::string& uri);
  HTTPResponse*  _autoindex(const std::string& path, LocationConfig& location);
  std::string _generateDirectoryListing(const std::string& path);
  std::string _getPath(const std::string& uri, LocationConfig& location);

 public:
  VirtualServer(ServerConfig& serverConfig);
  ~VirtualServer();
  bool        isDefaultServer();
  bool        isHostMatching(const std::string& host) const;
  HTTPResponse *checkRequest(HTTPRequest& request);
  std::string getServerName() const;
  HTTPResponse *handleRequest(HTTPRequest& request);
};

#endif
