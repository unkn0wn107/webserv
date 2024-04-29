#include "ConfigLoader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

ConfigLoader* ConfigLoader::instance = nullptr;

ConfigLoader& ConfigLoader::getInstance() {
  if (instance == nullptr) {
    instance = new ConfigLoader();
  }
  return *instance;
}

void ConfigLoader::loadConfig(const std::string& filename) {
  std::ifstream configFile(filename.c_str());
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
}

std::string ConfigLoader::getConfig(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator it = config.find(key);
  if (it != config.end()) {
    return it->second;
  } else {
    throw std::runtime_error("Configuration key not found: " + key);
  }
}

ConfigLoader::ConfigLoader() {
  // Default configuration values can be set here
  config["server_port"] = "8080";
  config["server_host"] = "127.0.0.1";
  config["max_client_body_size"] = "8192";  // 8 KB
}

ConfigLoader::~ConfigLoader() {
  // Cleanup if necessary
}
