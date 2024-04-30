/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:13 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:11:14 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLoader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

ConfigLoader* ConfigLoader::instance = NULL;

ConfigLoader* ConfigLoader::getInstance(const std::string* filepath) {
  if (!instance) {
    if (!filepath)
      instance = new ConfigLoader();
    else
      instance = new ConfigLoader(filepath);
  }
  return instance;
}

bool ConfigLoader::loadConfig(const std::string& filepath) {
  std::ifstream configFile(filepath.c_str());
  if (!configFile.is_open()) {
    throw std::runtime_error("Unable to open configuration file.");
  }

  std::string line;
  while (std::getline(configFile, line)) {
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
  // Default config
  config["server_port"] = "8080";
  config["server_host"] = "127.0.0.1";
  config["max_client_body_size"] = "8192";  // 8 KB
}

ConfigLoader::~ConfigLoader() {}
