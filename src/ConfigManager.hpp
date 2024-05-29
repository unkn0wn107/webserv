/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigManager.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:17 by agaley            #+#    #+#             */
/*   Updated: 2024/05/29 16:57:32 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "Config.hpp"
#include "Logger.hpp"

class ConfigManager {
 private:
  static ConfigManager* _instance;
  Config                _config;
  Logger&               _log;
  static std::string    _filePath;

  static void _printLocationsConfig(
      const std::map<std::string, LocationConfig*> locations);

  ConfigManager();

 public:
  ~ConfigManager();
  static ConfigManager& getInstance();
  const std::string     getFilePath() const;

  /**
   * Retrieve all config.
   * @return The configuration object.
   */
  const Config& getConfig() const;
  static void   printConfig();

  static void loadConfig(const std::string& filepath);

  static const std::string DEFAULT_FILE_NAME;
};

#endif
