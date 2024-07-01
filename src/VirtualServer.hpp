/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 10:22:15 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/28 10:32:43 by mchenava         ###   ########.fr       */
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
  ServerConfig&               _serverConfig;
  Logger&                     _log;
  bool                         _defaultServer;
  std::vector<std::string>    _hostNames;
  HTTPMethods*                _httpMethods;
  std::map<std::string, std::map<std::string, std::string>  > _sessionStore;

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
  void storeSessionData(const std::string& sessionId, const std::string& key, const std::string& value);
  std::string getSessionData(const std::string& sessionId, const std::string& key);
  void deleteSessionData(const std::string& sessionId, const std::string& key);
  void clearSessionData(const std::string& sessionId);
};

#endif
