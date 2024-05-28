/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:13 by agaley            #+#    #+#             */
/*   Updated: 2024/05/04 02:01:21 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLoader.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "Config.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

const std::string ConfigLoader::DEFAULT_HOST = "127.0.0.1";
const std::string ConfigLoader::DEFAULT_PORT = "8080";
const std::string ConfigLoader::DEFAULT_ROOT = "./site";
const std::string ConfigLoader::DEFAULT_MAX_CLIENT_BODY_SIZE = "8192";
const std::string ConfigLoader::DEFAULT_FILE_NAME = "server_config.cfg";
ConfigLoader*     ConfigLoader::_instance = NULL;

ConfigLoader& ConfigLoader::getInstance() {
  if (!_instance) {
    _instance = new ConfigLoader();
  }
  return *_instance;
}

void ConfigLoader::loadConfig(const std::string& filepath) {
  std::ifstream configFile(filepath.c_str());
  if (!configFile.is_open()) {
    throw std::runtime_error("Failed to open config file: " + filepath);
    getInstance()._log.error("Failed to open config file: " + filepath);
  }

  std::string line;
  while (getline(configFile, line)) {
    std::istringstream iss(line);
    std::string        key;
    std::string        value;
    iss >> key >> value;

    value = getInstance()._cleanValue(value, '#');
    if (key.empty() || key[0] == '#') {
      getInstance()._log.info("Skip empty or comment line :" + line);
      continue;
    }  // Skip comments and empty lines
    getInstance()._log.info("========== Parsing line : key = " + key +
                            " line = " + line);
    if (key == "server") {
      if (value == "{") {
        ServerConfig serverConfig;
        parseServerConfig(configFile, &serverConfig);
        getInstance()._config.servers.push_back(serverConfig);
        getInstance()._log.info(
            "++++++++++++++ new server config add to config");
      } else {
        throw std::runtime_error("Malformed server block");
      }
    } else if (key == "log_file") {
      getInstance()._config.log_file = getInstance()._parseValue(value);
    } else if (key == "worker_processes") {
      if (getInstance()._cleanValue(value, ';') == "auto")
        continue;
      else {
        getInstance()._config.worker_processes = Utils::stoi<int>(value);
      }
    } else
      getInstance()._log.warning("Unknowned key :" + key);
    key = "";
  }
  configFile.close();
  getInstance()._log.setConfig(getInstance()._config);
}

void ConfigLoader::parseServerConfig(std::ifstream& configFile,
                                     ServerConfig*  serverConfig) {
  std::string line;
  std::string key;
  while (key != "}" && getline(configFile, line)) {
    getInstance()._log.info("############### parseServerConfig line = [" +
                            line + "]");
    std::istringstream lineStream(line);
    std::string        value;
    std::string        afterValue;
    lineStream >> key >> value >> afterValue;

    if (key.empty() || key[0] == '#' || key == "}") {
      getInstance()._log.info("server block Skip empty or comment line :" +
                              line);
      continue;
    }

    afterValue = getInstance()._cleanValue(afterValue, '#');
    getInstance()._log.info("========= Parsing server block line: key = [" +
                            key + "] line = [" + line + "]");
    if (key == "location") {
      LocationConfig locationConfig;
      locationConfig.location = getInstance()._parseValue(lineStream.str());
      if (afterValue == "{") {
        parseLocationConfig(configFile, &locationConfig);
        serverConfig->locations.push_back(locationConfig);
        getInstance()._log.info(
            ">>>>>>>>>>>>>>>>>>>>> new route config add to server");
      } else {
        std::cerr << "Parsing error !" << std::endl;
        throw std::runtime_error("Malformed route block");
      }
    } else {
      if (key == "listen") {
        ListenConfig listenConfig;
        std::istringstream iss(line);
        parseListenConfig(iss, &listenConfig);
        serverConfig->listen.push_back(listenConfig);
        getInstance()._config.unique_listen_configs.insert(listenConfig);
      } else if (key == "server_name") {
        std::istringstream serverNames(line);
        std::string        serverName;
        serverNames >> serverName;
        while (serverNames >> serverName && serverName[0] != '#') {
          serverName = getInstance()._cleanValue(serverName, ';');
          serverConfig->server_names.push_back(serverName);
        }
      } else if (key == "root") {
        serverConfig->root = getInstance()._parseValue(lineStream.str());
      } else if (key == "client_max_body_size") {
        serverConfig->client_max_body_size =
            Utils::stoi<int>(getInstance()._parseValue(lineStream.str()));
      } else if (key == "error_page") {
        int         errorCode;
        std::string errorPage;
        errorCode = Utils::stoi<int>(value);
        getInstance()._log.info("Server block find error_page: errorCode = " +
                                Utils::to_string(errorCode) +
                                " errorPage = " + afterValue);
        errorPage = getInstance()._cleanValue(afterValue, ';');
        serverConfig->error_pages[errorCode] = errorPage;
      } else {
        getInstance()._log.warning("Server block find unknown key: key = " +
                                   key + " value = " + value);
      }
    }
    key = "";
  }

  if (serverConfig->root.empty())
    serverConfig->root = ConfigLoader::DEFAULT_ROOT;
}

void ConfigLoader::parseListenConfig(std::istringstream& lineStream, ListenConfig* listenConfig) {
  std::string token;

	while (lineStream >> token) {
    std::cout << "token = " << token << std::endl;
		if (token.find('[') != std::string::npos) { // Adresse IPv6 avec port
			std::string::size_type endPos = token.find(']');
			std::string::size_type colonPos = token.find(':', endPos);
			listenConfig->address = token.substr(1, endPos - 1); // Enlever les crochets
			if (colonPos != std::string::npos) {
				listenConfig->port = Utils::stoi<int>(token.substr(colonPos + 1));
			}
		} else if (token.find(':') != std::string::npos) { // Adresse IP ou nom d'hôte avec port
			std::string::size_type pos = token.find(':');
			listenConfig->address = token.substr(0, pos);
			listenConfig->port = Utils::stoi<int>(token.substr(pos + 1));
		} else if (std::isdigit(token[0])) { // Juste le port
			listenConfig->port = Utils::stoi<int>(token);
		} else if (token == "default_server") {
			listenConfig->default_server = true;
		} else if (token == "ipv6only=on") {
			listenConfig->ipv6only = true;
		} else if (token == "ipv6only=off") {
			listenConfig->ipv6only = false;
		} else if (token.find("backlog=") != std::string::npos) {
			listenConfig->backlog = Utils::stoi<int>(token.substr(8));
		} else if (token.find("rcvbuf=") != std::string::npos) {
			listenConfig->rcvbuf = Utils::stoi<int>(token.substr(7));
		} else if (token.find("sndbuf=") != std::string::npos) {
			listenConfig->sndbuf = Utils::stoi<int>(token.substr(7));
		}
	}
}

void ConfigLoader::parseLocationConfig(std::ifstream&  configFile,
                                       LocationConfig* locationConfig) {
  std::string line;
  std::string key;

  locationConfig->returnCode = 0;
  bool firstReturnEncountered = false;

  while (key != "}" && getline(configFile, line)) {
    std::istringstream lineStream(line);
    lineStream >> key;
    if (key.empty() || key[0] == '#' || key == "}") {
      getInstance()._log.info("route block Skip empty or comment line :" +
                              line);
      continue;
    }
    getInstance()._log.info(" ====== Parsing route block line: key = " + key +
                            " line = " + line);
    std::string value;
    if (key == "root") {
      locationConfig->root = getInstance()._parseValue(lineStream.str());
    } else if (key == "index") {
      locationConfig->index = getInstance()._parseValue(lineStream.str());
    } else if (key == "limit_except") {
      std::istringstream methods(line);
      std::string        method;
      methods >> method;
      methods >> method;
      while (methods >> method && method[0] != '{') {
        method = getInstance()._cleanValue(method, ' ');
        locationConfig->allow_methods.push_back(method);
      }
    } else if (key == "return") {
      if (firstReturnEncountered)
        continue;

      lineStream >> value;
      locationConfig->returnCode = Utils::stoi<int>(value);
      std::string            urlValue;
      std::string::size_type pos =
          line.find(' ', line.find(key) + key.length());
      if (pos != std::string::npos) {
        urlValue = line.substr(pos + 1);
      }
      locationConfig->returnUrl = getInstance()._parseValue(urlValue);
      firstReturnEncountered = true;
    } else if (key == "autoindex") {
      locationConfig->autoindex =
          (getInstance()._parseValue(lineStream.str()) == "on");
    } else if (key == "cgi") {
      locationConfig->cgi_handler = getInstance()._parseValue(lineStream.str());
    } else if (key == "accept_upload") {
      locationConfig->upload_path = getInstance()._parseValue(lineStream.str());
    }
    key = "";
  }
}

std::string ConfigLoader::_cleanValue(std::string toClean, char c) {
  size_t comPos = toClean.find(c);
  if (comPos != std::string::npos) {
    toClean = toClean.substr(0, comPos);
  }
  return toClean;
}

std::string ConfigLoader::_parseValue(std::string toParse) {
  std::istringstream toParseStream(toParse);
  std::string        key;
  std::string        value;
  std::string        afterValue;

  _log.info("Parsing value form :" + toParse);
  toParseStream >> key >> value >> afterValue;
  _log.info("parsed Value :" + value + " afterValue :" + afterValue);
  value = _cleanValue(value, '#');
  afterValue = _cleanValue(afterValue, '#');
  if (key == "route" && afterValue != "{")
    _log.warning("Badly terminated route line in server block: " + toParse +
                 " afterValue = " + afterValue);
  else if (key == "server" && value != "{")
    _log.warning("Badly terminated server line in server block: " + toParse +
                 " afterValue = " + afterValue);
  else {
    _log.info("key = " + key + " value = " + value +
              " afterValue = " + afterValue);
    size_t semicolonPos = value.find(';');
    if (semicolonPos != value.length()) {
      value = value.substr(0, semicolonPos);
    } else
      _log.warning("Badly terminated line in server block: " + toParse);
  }
  _log.info("Parsed value : " + value);
  return value;
}

std::string ConfigLoader::getServerConfigValue(const ServerConfig& config,
                                               const std::string&  key) const {
  if (key == "listen_port") {
    return Utils::to_string(config.listen[0].port);
  } else if (key == "server_name") {
    return config.server_names[0];
  } else if (key == "client_max_body_size") {
    return Utils::to_string(config.client_max_body_size);
  } else {
    _log.warning("Key not found in ServerConfig: " + key);
    return "";
  }
}

Config& ConfigLoader::getConfig() {
  return _instance->_config;
}

void ConfigLoader::printConfig() {
  const Config& config = getInstance()._config;
  std::cout << "Worker Processes: " << config.worker_processes << std::endl;
  std::cout << "Worker Connections: " << config.worker_connections << std::endl;
  std::cout << "Log File: " << config.log_file << std::endl;

  std::cout << "Unique Listen Configs:" << std::endl;
  for (std::set<ListenConfig>::const_iterator it = config.unique_listen_configs.begin(); it != config.unique_listen_configs.end(); ++it) {
    const ListenConfig& listen = *it;
    std::cout << "\tAddress: " << listen.address
              << " Port: " << listen.port
              << " Default Server: " << (listen.default_server ? "Yes" : "No")
              << " Backlog: " << listen.backlog
              << " Rcvbuf: " << listen.rcvbuf
              << " Sndbuf: " << listen.sndbuf
              << " IPv6 Only: " << (listen.ipv6only ? "Yes" : "No") << std::endl;
  }

  for (std::vector<ServerConfig>::const_iterator it = config.servers.begin(); it != config.servers.end(); ++it) {
    const ServerConfig& server = *it;
    std::cout << "====Server on port: " << server.listen[0].port << std::endl;
    std::cout << "Server Names: ";
    for (std::vector<std::string>::const_iterator nameIt = server.server_names.begin(); nameIt != server.server_names.end(); ++nameIt) {
      std::cout << *nameIt << " ";
    }
    std::cout << std::endl;
    std::cout << "Root: " << server.root << std::endl;
    std::cout << "Client Max Body Size: " << server.client_max_body_size << std::endl;

    std::cout << "====Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator errorIt = server.error_pages.begin(); errorIt != server.error_pages.end(); ++errorIt) {
      std::cout << " \tError Code: " << errorIt->first
                << " - Page: " << errorIt->second << std::endl;
    }

    std::cout << "Routes:" << std::endl;
    for (std::vector<LocationConfig>::const_iterator locationIt =
             server.locations.begin();
         locationIt != server.locations.end(); ++locationIt) {
      const LocationConfig& locationConfig = *locationIt;
      std::cout << "========Route: " << locationConfig.location << std::endl;
      std::cout << "\tDirectory: " << locationConfig.root << std::endl;
      std::cout << "\tDefault File: " << locationConfig.index << std::endl;
      std::cout << "\tReturn code: " << locationConfig.returnCode << std::endl;
      std::cout << "\tReturn url: " << locationConfig.returnUrl << std::endl;
      std::cout << "\tDirectory Listing: "
                << (locationConfig.autoindex ? "on" : "off") << std::endl;
      std::cout << "\tUpload Path: " << locationConfig.upload_path << std::endl;
      std::cout << "\tAllowed Methods: ";
      for (std::vector<std::string>::const_iterator methodIt =
               locationConfig.allow_methods.begin();
           methodIt != locationConfig.allow_methods.end(); ++methodIt) {
        std::cout << *methodIt << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

ConfigLoader::ConfigLoader() : _log(Logger::getInstance()) {}

ConfigLoader::~ConfigLoader() {}
