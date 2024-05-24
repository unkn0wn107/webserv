/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:17 by agaley            #+#    #+#             */
/*   Updated: 2024/05/24 04:13:04 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <fstream>
#include "Config.hpp"
#include "Logger.hpp"

class ConfigParser {
 private:
  static const Logger& _log;
  static std::string   _cleanValue(const std::string& toClean, char c);
  static std::string   _parseValue(const std::string& toParse);
  static void          _parseServerConfig(std::ifstream& configFile,
                                          ServerConfig*  serverConfig);
  static void          _parseLocationConfig(std::ifstream&  configFile,
                                            LocationConfig* locationConfig);
  static void          _insertLocationConfig(TrieNode*          node,
                                             const std::string& path,
                                             LocationConfig*    config);

 public:
  static Config parseConfigFile(const std::string& filepath);

  static const std::string DEFAULT_HOST;
  static const std::string DEFAULT_PORT;
  static const std::string DEFAULT_ROOT;
  static const std::string DEFAULT_MAX_CLIENT_BODY_SIZE;
};

#endif
