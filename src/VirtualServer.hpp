/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 10:22:15 by  mchenava         #+#    #+#             */
/*   Updated: 2024/07/02 23:44:22 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUAL_SERVER_HPP
#define VIRTUAL_SERVER_HPP

#include "CGIHandler.hpp"
#include "Config.hpp"
#include "HTTPMethods.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"

class HTTPMethods;

class VirtualServer {
 private:
  const ServerConfig&                                        _serverConfig;
  Logger&                                                    _log;
  bool                                                       _defaultServer;
  std::vector<std::string>                                   _hostNames;
  HTTPMethods*                                               _httpMethods;
  std::map<std::string, std::map<std::string, std::string> > _sessionStore;

  bool                       _hasDefaultListenConfig();
  std::map<int, std::string> _getErrorPages(const std::string& uri);

 public:
  VirtualServer(const ServerConfig& serverConfig);
  ~VirtualServer();
  bool                  isDefaultServer();
  bool                  isHostMatching(const std::string& host) const;
  HTTPResponse*         checkRequest(HTTPRequest& request);
  std::string           getServerName() const;
  HTTPResponse*         handleRequest(HTTPRequest& request);
  const LocationConfig& getLocationConfig(const std::string& uri) const;
  std::string           getRoot() const;
  void                  storeSessionData(const std::string& sessionId,
                                         const std::string& key,
                                         const std::string& value);
  std::string           getSessionData(const std::string& sessionId,
                                       const std::string& key);
  void deleteSessionData(const std::string& sessionId, const std::string& key);
  void clearSessionData(const std::string& sessionId);
};

#endif
