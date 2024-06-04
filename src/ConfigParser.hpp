/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:17 by agaley            #+#    #+#             */
/*   Updated: 2024/06/04 13:39:39 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "Config.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

class ConfigParser {
  public:
    static Config parseConfigFile(const std::string& filepath);

  private:
    static std::string _configFilepath;

    static void   _parseServerConfig(std::ifstream& configFile,
                                    ServerConfig&  serverConfig,
                                    Config&        config);
    static void _parseListenConfig(std::istringstream& lineStream,
                                    ListenConfig*       listenConfig);
    static void _parseLocationConfig(std::ifstream&  configFile,
                                      LocationConfig& locationConfig,
                                      ServerConfig&   serverConfig);
    static void _fillLocationDefinedByServerConfig(
        LocationConfig& locationConfig, ServerConfig& serverConfig);

    static std::string              _cleanValue(std::string toClean, char c);
    static std::string              _parseValue(std::string toParse);
    static std::string              _trim(const std::string& str);
    static std::vector<std::string> _split(const std::string& str,
                                            char               delimiter);

    static const Logger&     _log;
    static const std::string DEFAULT_HOST;
    static const std::string DEFAULT_PORT;
    static const std::string DEFAULT_ROOT;
    static const std::string DEFAULT_MAX_CLIENT_BODY_SIZE;
};

#endif
