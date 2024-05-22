/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:17 by agaley            #+#    #+#             */
/*   Updated: 2024/05/22 18:00:35 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGLOADER_H
#define CONFIGLOADER_H

#include <map>
#include <string>
#include "Config.hpp"
#include "Logger.hpp"

/// @brief
class ConfigLoader {
 private:
  ConfigLoader();
  static ConfigLoader* _instance;
  Config               _config;
  Logger&              _log;

  // Disallow the copy constructor and copy assignment operator
  ConfigLoader(const ConfigLoader&);
  ConfigLoader& operator=(const ConfigLoader&);
  std::string   _parseValue(std::string value);
  std::string   _cleanValue(std::string toClean, char c);

 public:
  static ConfigLoader& getInstance();
  ~ConfigLoader();

  /**
   * Load configuration from a file.
   * @param filepath Path to the configuration file.
   * @return True if the configuration is loaded successfully, false otherwise.
   */
  static void loadConfig(const std::string& filepath);
  static void parseServerConfig(std::ifstream& configFile,
                                ServerConfig*  serverConfig);
  static void parseLocationConfig(std::ifstream&  configFile,
                                  LocationConfig* locationConfig);
  std::string getServerConfigValue(const ServerConfig& config,
                                   const std::string&  key) const;

  /**
   * Retrieve all config.
   * @return The configuration map.
   */
  const Config& getConfig() const;
  static void   printConfig();

  static const std::string DEFAULT_HOST;
  static const std::string DEFAULT_PORT;
  static const std::string DEFAULT_ROOT;
  static const std::string DEFAULT_MAX_CLIENT_BODY_SIZE;
  static const std::string DEFAULT_FILE_NAME;
};

#endif
