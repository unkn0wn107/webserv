/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:13 by agaley            #+#    #+#             */
/*   Updated: 2024/05/04 02:01:21 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLoader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

const std::string ConfigLoader::DEFAULT_HOST = "127.0.0.1";
const std::string ConfigLoader::DEFAULT_PORT = "8080";
const std::string ConfigLoader::DEFAULT_ROOT = "./";
const std::string ConfigLoader::DEFAULT_MAX_CLIENT_BODY_SIZE = "8192";
const std::string ConfigLoader::DEFAULT_FILE_NAME = "server_config.cfg";
ConfigLoader*     ConfigLoader::instance = NULL;

ConfigLoader& ConfigLoader::getInstance() {
  if (!instance) {
    instance = new ConfigLoader();
  }
  return *instance;
}

bool ConfigLoader::loadConfig(const std::string& filepath) {
  std::ifstream configFile(filepath.c_str());
  if (!configFile.is_open()) {
    throw std::runtime_error("Unable to open configuration file.");
  }

  std::string line;
  while (std::getline(configFile, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    std::istringstream iss(line);
    std::string        key;
    if (std::getline(iss, key, '=')) {
      std::string value;
      if (std::getline(iss, value)) {
        config[key] = value;
      }
    }
  }
  configFile.close();
  return true;
}

std::string ConfigLoader::getConfigValue(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator it = config.find(key);
  if (it != config.end()) {
    return it->second;
  } else {
    throw std::runtime_error("Configuration key not found: " + key);
  }
}

std::map<std::string, std::string> ConfigLoader::getConfig() const {
  return config;
}

ConfigLoader::ConfigLoader() {
  config["host"] = DEFAULT_HOST;
  config["port"] = DEFAULT_PORT;
  config["root"] = DEFAULT_ROOT;
  config["max_client_body_size"] = DEFAULT_MAX_CLIENT_BODY_SIZE;
}

ConfigLoader::~ConfigLoader() {}
