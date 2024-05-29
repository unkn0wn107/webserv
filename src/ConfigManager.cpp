/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigManager.cpp                                   :+:      :+:    :+: */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:13 by agaley            #+#    #+#             */
/*   Updated: 2024/05/04 02:01:21 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigManager.hpp"
#include "ConfigParser.hpp"
#include "Logger.hpp"

const std::string ConfigManager::DEFAULT_FILE_NAME = "default.conf";
ConfigManager*    ConfigManager::_instance = NULL;
std::string       ConfigManager::_filePath = "";

ConfigManager::ConfigManager() : _log(Logger::getInstance()) {}

ConfigManager::~ConfigManager() {}

ConfigManager& ConfigManager::getInstance() {
  if (!_instance) {
    _instance = new ConfigManager();
  }
  return *_instance;
}

const Config& ConfigManager::getConfig() const {
  return _instance->_config;
}

const std::string ConfigManager::getFilePath() const {
  return _filePath;
}

void ConfigManager::loadConfig(const std::string& filepath) {
  ConfigManager::_filePath = filepath;
  ConfigManager::getInstance()._config =
      ConfigParser::parseConfigFile(filepath);
  ConfigManager::getInstance()._log.setConfig(
      ConfigManager::getInstance()._config);
}

void ConfigManager::printConfig() {
  const Config& config = getInstance()._config;
  for (std::vector<ServerConfig>::const_iterator it = config.servers.begin();
       it != config.servers.end(); ++it) {
    const ServerConfig& server = *it;
    std::cout << "====Server on port: " << server.listen_port << std::endl;
    std::cout << "Server Names: ";
    for (std::vector<std::string>::const_iterator nameIt =
             server.server_names.begin();
         nameIt != server.server_names.end(); ++nameIt) {
      std::cout << *nameIt << " ";
    }
    std::cout << std::endl;
    std::cout << "Root: " << server.root << std::endl;
    std::cout << "Client Max Body Size: " << server.client_max_body_size
              << std::endl;

    // Display error pages
    std::cout << "====Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator errorIt =
             server.error_pages.begin();
         errorIt != server.error_pages.end(); ++errorIt) {
      std::cout << " \tError Code: " << errorIt->first
                << " - Page: " << errorIt->second << std::endl;
    }

    std::cout << "Routes:" << std::endl;
    _printLocationsConfig(server.locations);
  }
}

void ConfigManager::_printLocationsConfig(
    const std::map<std::string, LocationConfig*> locations) {
  for (std::map<std::string, LocationConfig*>::const_iterator it =
           locations.begin();
       it != locations.end(); ++it) {
    const LocationConfig& locationConfig = *it->second;
    std::cout << "========Route: " << it->first << std::endl;
    std::cout << "\tDirectory: " << locationConfig.root << std::endl;
    std::cout << "\tDefault File: " << locationConfig.index << std::endl;
    std::cout << "\tReturn code: " << locationConfig.returnCode << std::endl;
    std::cout << "\tReturn url: " << locationConfig.returnUrl << std::endl;
    std::cout << "\tDirectory Listing: "
              << (locationConfig.autoindex ? "on" : "off") << std::endl;
    std::cout << "\tUpload Path: " << locationConfig.upload_path << std::endl;
    std::cout << "\tAllowed Methods: ";
    for (std::vector<std::string>::const_iterator methodIt =
             locationConfig.allowed_methods.begin();
         methodIt != locationConfig.allowed_methods.end(); ++methodIt) {
      std::cout << *methodIt << " ";
    }
    std::cout << std::endl;
  }
}
