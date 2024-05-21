/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPProtocol.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:40 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 20:03:13 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPROTOCOL_H
#define HTTPPROTOCOL_H

#include <string>
#include "Logger.hpp"
#include "Config.hpp"

class HTTPRequest;
class HTTPResponse;

// enum Version { UNKNOWN, HTTP1_1 };

class HTTPProtocol {
  protected:
    Logger&         _log;
    ServerConfig&   _config;
  public:
    HTTPProtocol(ServerConfig& config): _log(Logger::getInstance()), _config(config) {}
    virtual ~HTTPProtocol() {}

    // Version version;

    virtual HTTPRequest  parseRequest(const std::string& requestString) = 0;
    virtual HTTPResponse processRequest(const HTTPRequest& request) = 0;
};

#endif
