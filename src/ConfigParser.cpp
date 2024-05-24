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
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "Config.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

const std::string ConfigParser::DEFAULT_HOST = "127.0.0.1";
const std::string ConfigParser::DEFAULT_PORT = "8080";
const std::string ConfigParser::DEFAULT_ROOT = "./site";
const std::string ConfigParser::DEFAULT_MAX_CLIENT_BODY_SIZE = "8192";

const Logger& ConfigParser::_log = Logger::getInstance();

Config ConfigParser::parseConfigFile(const std::string& filepath) {
  std::ifstream configFile(filepath.c_str());
  if (!configFile.is_open()) {
    throw std::runtime_error("Failed to open config file: " + filepath);
  }

  Config      config;
  std::string line;
  while (getline(configFile, line)) {
    std::istringstream iss(line);
    std::string        key, value;
    iss >> key >> value;

    value = _cleanValue(value, '#');
    if (key.empty() || key[0] == '#') {
      continue;
    }

    if (key == "server") {
      if (value == "{") {
        ServerConfig serverConfig;
        _parseServerConfig(configFile, &serverConfig);
        config.servers.push_back(serverConfig);
      } else {
        throw std::runtime_error("Malformed server block");
      }
      // } else if (key == "log_file") {
      //   _log.setFile(value);
    } else {
      _log.emerg("Unknown directive: \"" + key + "\" in " + _log.getFileName());
    }
  }
  configFile.close();
  return config;
}

void ConfigParser::_parseServerConfig(std::ifstream& configFile,
                                      ServerConfig*  serverConfig) {
  std::string line;
  std::string key;
  while (key != "}" && getline(configFile, line)) {
    std::istringstream lineStream(line);
    std::string        value;
    std::string        afterValue;
    lineStream >> key >> value >> afterValue;

    afterValue = _cleanValue(afterValue, '#');
    if (key.empty() || key[0] == '#' || key == "}") {
      continue;
    }

    if (key == "location") {
      LocationConfig* locationConfig = new LocationConfig();
      _parseLocationConfig(configFile, locationConfig);
      _insertLocationConfig(serverConfig->locationTrie, value, locationConfig);
    } else {
      if (key == "listen") {
        serverConfig->listen_port =
            Utils::stoi<int>(_parseValue(lineStream.str()));
      } else if (key == "server_name") {
        std::istringstream serverNames(line);
        std::string        serverName;
        while (serverNames >> serverName && serverName[0] != '#') {
          serverName = _cleanValue(serverName, ';');
          if (serverName != "server_name")
            serverConfig->server_names.push_back(serverName);
        }
      } else if (key == "root") {
        serverConfig->root = _parseValue(lineStream.str());
      } else if (key == "client_max_body_size") {
        serverConfig->client_max_body_size =
            Utils::stoi<int>(_parseValue(lineStream.str()));
      } else if (key == "error_page") {
        int         errorCode;
        std::string errorPage;
        errorCode = Utils::stoi<int>(value);
        errorPage = _cleanValue(afterValue, ';');
        serverConfig->error_pages[errorCode] = errorPage;
      } else {
        _log.emerg("Unknown directive: \"" + key + "\" in " +
                   _log.getFileName());
      }
      key = "";
    }

    if (serverConfig->root.empty())
      serverConfig->root = DEFAULT_ROOT;
  }
}

void ConfigParser::_parseLocationConfig(std::ifstream&  configFile,
                                        LocationConfig* locationConfig) {
  std::string line;
  std::string key;
  while (
      getline(configFile, line) &&
      line.find('}') ==
          std::string::npos) {  // Continue until the end of the location block
    std::istringstream lineStream(line);
    std::string        value;
    lineStream >> key >> value;

    value = _cleanValue(value, '#');  // Clean comments
    if (key.empty() || key[0] == '#') {
      continue;  // Skip comments or empty lines
    }

    if (key == "root") {
      locationConfig->root = _parseValue(lineStream.str());
    } else if (key == "index") {
      locationConfig->index = _parseValue(lineStream.str());
    } else if (key == "limit_except") {
      std::string method;
      while (lineStream >> method && method != "{") {
        if (method.find(';') != std::string::npos) {
          method = _cleanValue(method, ';');
        }
        locationConfig->allowed_methods.push_back(method);
      }
    } else if (key == "autoindex") {
      std::string flag = _parseValue(lineStream.str());
      locationConfig->autoindex = (flag == "on");
    } else if (key == "return") {
      int         statusCode = Utils::stoi<int>(value);
      std::string url = _cleanValue(lineStream.str(), ';');
      locationConfig->returnCode = statusCode;
      locationConfig->returnUrl = url;
    } else {
      _log.emerg("Unknown directive in location block: \"" + key + "\" in " +
                 _log.getFileName());
    }
  }
}

// TODO : Dependency injection : insert => Trie struct
void ConfigParser::_insertLocationConfig(TrieNode*          node,
                                         const std::string& path,
                                         LocationConfig*    config) {
  TrieNode* current = node;
  for (size_t i = 0; i < path.length(); ++i) {
    char ch = path[i];
    if (current->children.find(ch) == current->children.end()) {
      current->children[ch] = new TrieNode();
    }
    current = current->children[ch];
  }
  current->locationConfig = config;
}

std::string ConfigParser::_cleanValue(const std::string& toClean, char c) {
  size_t comPos = toClean.find(c);
  return comPos != std::string::npos ? toClean.substr(0, comPos) : toClean;
}

std::string ConfigParser::_parseValue(const std::string& toParse) {
  std::istringstream toParseStream(toParse);
  std::string        key, value, afterValue;
  toParseStream >> key >> value >> afterValue;
  value = _cleanValue(value, '#');
  size_t semicolonPos = value.find(';');
  return semicolonPos != std::string::npos ? value.substr(0, semicolonPos)
                                           : value;
}
