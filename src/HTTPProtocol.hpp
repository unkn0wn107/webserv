/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPProtocol.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:40 by agaley            #+#    #+#             */
/*   Updated: 2024/06/28 01:53:04 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPROTOCOL_H
#define HTTPPROTOCOL_H

#include <string>
#include "Config.hpp"
#include "Logger.hpp"

class HTTPRequest;
class HTTPResponse;

// enum Version { UNKNOWN, HTTP1_1 };

class HTTPProtocol {
 protected:
  Logger&       _log;
  ServerConfig _config;

 public:
  HTTPProtocol(ServerConfig config)
      : _log(Logger::getInstance()), _config(config) {}
  virtual ~HTTPProtocol() {}

  // Version version;

  virtual HTTPRequest  parseRequest(const std::string& requestString) = 0;
  virtual HTTPResponse processRequest(const HTTPRequest& request) = 0;
};

#endif
