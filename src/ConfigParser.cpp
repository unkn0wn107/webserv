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
const std::string ConfigParser::DEFAULT_INDEX = "index.html";

std::string ConfigParser::_configFilepath = "";

const Logger& ConfigParser::_log = Logger::getInstance();

Config ConfigParser::parseConfigFile(const std::string& filepath) {
  _configFilepath = filepath;
  std::ifstream configFile(filepath.c_str());
  if (!configFile.is_open()) {
    _log.emerg("Failed to open config file: " + filepath);
    throw Exception("Failed to open config file: " + filepath);
  }

  Config      config;
  std::string line;
  while (getline(configFile, line)) {
    std::istringstream iss(line);
    std::string        key;
    std::string        value;
    iss >> key >> value;

    value = _cleanValue(value, '#');
    if (key.empty() || key[0] == '#')
      continue;
    if (key == "server") {
      if (value == "{") {
        ServerConfig serverConfig;
        _parseServerConfig(configFile, serverConfig, config);
        config.servers.push_back(serverConfig);
      } else {
        throw Exception("Malformed server block");
      }
    } else if (key == "log_file") {
      config.log_file = _parseValue(value);
    } else if (key == "worker_processes") {
      if (_cleanValue(value, ';') == "auto")
        continue;
      else {
        config.worker_processes = Utils::stoi<int>(value);
      }
    } else
      _log.emerg("Unknown config directive: \"" + key + "\" in " +
                 _configFilepath);
    key = "";
  }
  configFile.close();
  return config;
}

void ConfigParser::_parseServerConfig(std::ifstream& configFile,
                                      ServerConfig&  serverConfig,
                                      Config&        config) {
  std::string line;
  std::string key;
  while (key != "}" && getline(configFile, line)) {
    std::istringstream lineStream(line);
    std::string        value;
    std::string        afterValue;
    lineStream >> key >> value >> afterValue;

    if (key.empty() || key[0] == '#' || key == "}") {
      continue;
    }

    afterValue = _cleanValue(afterValue, '#');
    if (key == "location") {
      LocationConfig locationConfig;
      locationConfig.location = _parseValue(lineStream.str());
      if (afterValue == "{") {
        _parseLocationConfig(configFile, locationConfig, serverConfig);
        serverConfig.locations[value] = locationConfig;
      } else {
        std::cerr << "Parsing error !" << std::endl;
        throw Exception("Malformed route block");
      }
    } else {
      if (key == "listen") {
        ListenConfig       listenConfig;
        std::istringstream iss(line);
        _parseListenConfig(iss, &listenConfig);
        serverConfig.listen.push_back(listenConfig);
        config.unique_listen_configs.insert(listenConfig);
      } else if (key == "server_name") {
        std::istringstream serverNames(line);
        std::string        serverName;
        serverNames >> serverName;
        while (serverNames >> serverName && serverName[0] != '#') {
          serverName = _cleanValue(serverName, ';');
          serverConfig.server_names.push_back(serverName);
        }
      } else if (key == "root") {
        serverConfig.root = _parseValue(lineStream.str());
      } else if (key == "index") {
        serverConfig.index = _parseValue(lineStream.str());
      } else if (key == "autoindex") {
        serverConfig.autoindex = (_parseValue(lineStream.str()) == "on");
      } else if (key == "cgi") {
        serverConfig.cgi = (_parseValue(lineStream.str()) == "on");
      } else if (key == "accept_upload") {
        serverConfig.upload = (_parseValue(lineStream.str()) == "on");
      } else if (key == "accept_delete") {
        serverConfig.delete_ = (_parseValue(lineStream.str()) == "on");
      } else if (key == "client_max_body_size") {
        serverConfig.client_max_body_size =
            Utils::stoi<int>(_parseValue(lineStream.str()));
      } else if (key == "error_page") {
        int         errorCode;
        std::string errorPage;
        errorCode = Utils::stoi<int>(value);
        errorPage = _cleanValue(afterValue, ';');
        serverConfig.error_pages[errorCode] = errorPage;
      } else {
        _log.emerg("Unknown server directive: \"" + key + "\" in " +
                   _configFilepath);
      }
    }
    key = "";
  }

  if (serverConfig.root.empty())
    serverConfig.root = ConfigParser::DEFAULT_ROOT;
}

void ConfigParser::_parseListenConfig(std::istringstream& lineStream,
                                      ListenConfig*       listenConfig) {
  std::string token;

  while (lineStream >> token) {
    std::cout << "token = " << token << std::endl;
    if (token.find('[') != std::string::npos) {  // Adresse IPv6 avec port
      std::string::size_type endPos = token.find(']');
      std::string::size_type colonPos = token.find(':', endPos);
      listenConfig->address =
          token.substr(1, endPos - 1);  // Enlever les crochets
      if (colonPos != std::string::npos) {
        listenConfig->port = Utils::stoi<int>(token.substr(colonPos + 1));
      }
    } else if (token.find(':') !=
               std::string::npos) {  // Adresse IP ou nom d'hôte avec port
      std::string::size_type pos = token.find(':');
      listenConfig->address = token.substr(0, pos);
      listenConfig->port = Utils::stoi<int>(token.substr(pos + 1));
    } else if (std::isdigit(token[0])) {  // Juste le port
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

void ConfigParser::_fillLocationDefinedByServerConfig(
    LocationConfig& locationConfig,
    ServerConfig&   serverConfig) {
  locationConfig.root =
      serverConfig.root.empty() ? DEFAULT_ROOT : serverConfig.root;
  locationConfig.index =
      serverConfig.index.empty() ? DEFAULT_INDEX : serverConfig.index;
  locationConfig.client_max_body_size =
      serverConfig.client_max_body_size == 0
          ? Utils::stoi<unsigned int>(DEFAULT_MAX_CLIENT_BODY_SIZE)
          : serverConfig.client_max_body_size;
  locationConfig.autoindex = serverConfig.autoindex;
  locationConfig.cgi = serverConfig.cgi;
  locationConfig.upload = serverConfig.upload;
  locationConfig.error_pages = serverConfig.error_pages;
}

void ConfigParser::_parseLocationConfig(std::ifstream&  configFile,
                                        LocationConfig& locationConfig,
                                        ServerConfig&   serverConfig) {
  std::string line;
  std::string key;
  std::string value;
  std::string afterValue;

  locationConfig.returnCode = 0;
  bool firstReturnEncountered = false;
  bool limitExceptEncountered = false;
  _fillLocationDefinedByServerConfig(locationConfig, serverConfig);

  while (key != "}" && getline(configFile, line)) {
    std::istringstream lineStream(line);
    lineStream >> key >> value >> afterValue;
    if (key.empty() || key[0] == '#' || key == "}")
      continue;
    if (key == "root") {
      locationConfig.root = _parseValue(lineStream.str());
    } else if (key == "index") {
      locationConfig.index = _parseValue(lineStream.str());
    } else if (key == "limit_except") {
      std::istringstream methods(line);
      std::string        method;
      methods >> method;
      while (methods >> method && method[0] != '{') {
        method = _cleanValue(method, ' ');
        locationConfig.allowed_methods.insert(method);
        if (method == "GET")
          locationConfig.allowed_methods.insert("HEAD");
      }
      limitExceptEncountered = true;
    } else if (key == "return") {
      if (firstReturnEncountered)
        continue;
      locationConfig.returnCode = Utils::stoi<int>(value);
      std::string            urlValue;
      std::string::size_type pos =
          line.find(' ', line.find(key) + key.length());
      if (pos != std::string::npos) {
        urlValue = line.substr(pos + 1);
      }
      locationConfig.returnUrl = _parseValue(urlValue);
      firstReturnEncountered = true;
    } else if (key == "autoindex") {
      locationConfig.autoindex = (_parseValue(lineStream.str()) == "on");
    } else if (key == "cgi") {
      locationConfig.cgi = (_parseValue(lineStream.str()) == "on");
    } else if (key == "upload") {
      locationConfig.upload = (_parseValue(lineStream.str()) == "on");
    } else if (key == "delete") {
      locationConfig.delete_ = (_parseValue(lineStream.str()) == "on");
    } else if (key == "client_max_body_size") {
      locationConfig.client_max_body_size =
          Utils::stoi<int>(_parseValue(lineStream.str()));
    } else if (key == "error_page") {
      int         errorCode;
      std::string errorPage;
      errorCode = Utils::stoi<int>(value);
      errorPage = _cleanValue(afterValue, ';');
      locationConfig.error_pages[errorCode] = errorPage;
    } else {
      _log.emerg("Unknown location directive: \"" + key + "\" in " +
                 _configFilepath);
    }
    key = "";
  }
  if (!limitExceptEncountered) {
    for (size_t i = 0; i < sizeof(HTTPRequest::supportedMethods) /
                               sizeof(HTTPRequest::supportedMethods[0]);
         ++i) {
      locationConfig.allowed_methods.insert(HTTPRequest::supportedMethods[i]);
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

std::string ConfigParser::_cleanValue(std::string toClean, char c) {
  size_t comPos = toClean.find(c);
  if (comPos != std::string::npos) {
    toClean = toClean.substr(0, comPos);
  }
  return toClean;
}

std::string ConfigParser::_parseValue(std::string toParse) {
  std::istringstream toParseStream(toParse);
  std::string        key;
  std::string        value;
  std::string        afterValue;

  toParseStream >> key >> value >> afterValue;
  value = _cleanValue(value, '#');
  afterValue = _cleanValue(afterValue, '#');
  if (key == "route" && afterValue != "{")
    _log.warning("Badly terminated route line in server block: " + toParse +
                 " afterValue = " + afterValue);
  else if (key == "server" && value != "{")
    _log.warning("Badly terminated server line in server block: " + toParse +
                 " afterValue = " + afterValue);
  else {
    size_t semicolonPos = value.find(';');
    if (semicolonPos != value.length()) {
      value = value.substr(0, semicolonPos);
    } else
      _log.warning("Badly terminated line in server block: " + toParse);
  }
  return value;
}
