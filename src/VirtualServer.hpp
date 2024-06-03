/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 10:22:15 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/03 15:12:37 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUAL_SERVER_HPP
#define VIRTUAL_SERVER_HPP

#include "Config.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"

class VirtualServer {
 private:
  ServerConfig&            _serverConfig;
  Logger&                  _log;
  bool                     _defaultServer;
  std::vector<std::string>    _hostNames;
  static std::map<int, std::string>  _defaultErrorPages;

  bool            _hasDefaultListenConfig();
  LocationConfig& _getLocationConfig(const std::string& uri);
  HTTPResponse *_handleGetRequest(HTTPRequest& request);
  std::map<int, std::string> _getErrorPages(const std::string& uri);
  std::string     _fillErrorPage(int status, const std::string& path);
  static void _initializeErrorPages();
  HTTPResponse*  _autoindex(const std::string& path, LocationConfig& location, const std::string& uri);
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
  static std::string defaultErrorPage(int status);
};

#endif
