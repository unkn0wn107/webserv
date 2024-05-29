/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:13 by agaley            #+#    #+#             */
/*   Updated: 2024/05/04 02:01:21 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

const std::string ConfigParser::DEFAULT_HOST = "127.0.0.1";
const std::string ConfigParser::DEFAULT_PORT = "8080";
const std::string ConfigParser::DEFAULT_ROOT = "./site";
const std::string ConfigParser::DEFAULT_MAX_CLIENT_BODY_SIZE = "8192";

std::string ConfigParser::_configFilepath = "";

const Logger& ConfigParser::_log = Logger::getInstance();

Config ConfigParser::parseConfigFile(const std::string& filepath) {
  _configFilepath = filepath;
  std::ifstream configFile(filepath.c_str());
  if (!configFile.is_open()) {
    throw std::runtime_error("Failed to open config file: " + filepath);
  }

  Config      config;
  std::string line;
  while (getline(configFile, line)) {
    line = _trim(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::vector<std::string> tokens = _split(line, ' ');
    std::string              key = tokens[0];
    std::string              value = tokens.size() > 1 ? tokens[1] : "";

    if (key == "server") {
      if (value == "{") {
        ServerConfig serverConfig;
        _parseServerConfig(configFile, serverConfig);
        config.servers.push_back(serverConfig);
      } else {
        throw std::runtime_error("Malformed server block");
      }
    } else {
      _log.emerg("Unknown directive: \"" + key + "\" in " + filepath);
    }
  }
  configFile.close();
  return config;
}

void ConfigParser::_parseServerConfig(std::ifstream& configFile,
                                      ServerConfig&  serverConfig) {
  std::string line;
  while (getline(configFile, line) && line != "}") {
    line = _trim(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::vector<std::string> tokens = _split(line, ' ');
    std::string              key = tokens[0];
    std::string              value = tokens.size() > 1 ? tokens[1] : "";

    if (key == "listen") {
      serverConfig.listen_port = Utils::stoi<int>(value);
    } else if (key == "server_name") {
      std::vector<std::string> serverNames = _split(value, ' ');
      for (std::vector<std::string>::const_iterator it = serverNames.begin();
           it != serverNames.end(); ++it) {
        const std::string& serverName = *it;
        if (!serverName.empty()) {
          serverConfig.server_names.push_back(serverName);
        }
      }
    } else if (key == "root") {
      serverConfig.root = value;
    } else if (key == "autoindex") {
      serverConfig.autoindex = (value == "on");
    } else if (key == "client_max_body_size") {
      serverConfig.client_max_body_size = Utils::stoi<int>(value);
    } else if (key == "error_page") {
      std::vector<std::string> errorTokens = _split(value, ' ');
      if (errorTokens.size() == 2) {
        int         errorCode = Utils::stoi<int>(errorTokens[0]);
        std::string errorPage = errorTokens[1];
        serverConfig.error_pages[errorCode] = errorPage;
      }
    } else if (key == "location") {
      if (value == "{") {
        LocationConfig locationConfig;
        _parseLocationConfig(configFile, locationConfig);
        serverConfig.locations[value] = &locationConfig;
      } else {
        throw std::runtime_error("Malformed location block");
      }
    } else {
      _log.emerg("Unknown directive: \"" + key + "\" in " + _configFilepath);
    }
  }

  if (serverConfig.root.empty()) {
    serverConfig.root = DEFAULT_ROOT;
  }
}

void ConfigParser::_parseLocationConfig(std::ifstream&  configFile,
                                        LocationConfig& locationConfig) {
  std::string line;
  while (getline(configFile, line) && line != "}") {
    line = _trim(line);
    if (line.empty() || line[0] == '#') {
      continue;
    }

    std::vector<std::string> tokens = _split(line, ' ');
    std::string              key = tokens[0];
    std::string              value = tokens.size() > 1 ? tokens[1] : "";

    if (key == "location") {
      locationConfig.location = value;
    } else if (key == "root") {
      locationConfig.root = value;
    } else if (key == "index") {
      locationConfig.index = value;
    } else if (key == "limit_except") {
      std::vector<std::string> methods = _split(value, ' ');
      for (std::vector<std::string>::const_iterator it = methods.begin();
           it != methods.end(); ++it) {
        const std::string& method = *it;
        if (!method.empty()) {
          locationConfig.allowed_methods.push_back(method);
        }
      }
    } else if (key == "autoindex") {
      locationConfig.autoindex = (value == "on");
    } else if (key == "return") {
      std::vector<std::string> returnTokens = _split(value, ' ');
      if (returnTokens.size() == 2) {
        int         statusCode = Utils::stoi<int>(returnTokens[0]);
        std::string url = returnTokens[1];
        locationConfig.returnCode = statusCode;
        locationConfig.returnUrl = url;
      }
    } else {
      _log.emerg("Unknown directive in location block: \"" + key + "\" in " +
                 _configFilepath);
    }
  }
}

std::string ConfigParser::_trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t\n\r\f");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = str.find_last_not_of(" \t\n\r\f");
  return str.substr(start, end - start + 1);
}

std::vector<std::string> ConfigParser::_split(const std::string& str,
                                              char               delimiter) {
  std::stringstream        ss(str);
  std::string              item;
  std::vector<std::string> tokens;
  while (getline(ss, item, delimiter)) {
    tokens.push_back(item);
  }
  return tokens;
}
