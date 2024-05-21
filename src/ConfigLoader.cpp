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
#include "Config.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <iostream>

const std::string ConfigLoader::DEFAULT_HOST = "127.0.0.1";
const std::string ConfigLoader::DEFAULT_PORT = "8080";
const std::string ConfigLoader::DEFAULT_ROOT = "./";
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
        std::string key;
        std::string value;
        iss >> key >> value;

        value = getInstance()._cleanValue(value, '#');
        if (key.empty() || key[0] == '#') {
            getInstance()._log.info("Skip empty or comment line :" + line);
            continue;
        } // Skip comments and empty lines
        getInstance()._log.info("========== Parsing line : key = " + key + " line = " + line);
        if (key == "server") {
            if (value == "{") {
                ServerConfig serverConfig;
                parseServerConfig(configFile, &serverConfig);
                getInstance()._config.servers.push_back(serverConfig);
                getInstance()._log.info("++++++++++++++ new server config add to config");
            } else {
                throw std::runtime_error("Malformed server block");
            }
        } else if (key == "log_file") {
            getInstance()._config.log_file = getInstance()._parseValue(value);
        }
        else
            getInstance()._log.warning("Unknowned key :"+ key);
        key = "";
    }
    configFile.close();
    getInstance()._log.setConfig(getInstance()._config);
}

void ConfigLoader::parseServerConfig(std::ifstream& configFile, ServerConfig* serverConfig) {
    std::string line;
    std::string key;
    while (key != "}" && getline(configFile, line)) {
        getInstance()._log.info("############### parseServerConfig line = [" + line + "]");
        std::istringstream lineStream(line);
        std::string value;
        std::string afterValue;
        lineStream >> key >> value >> afterValue;

        if (key.empty() || key[0] == '#' || key == "}") {
            getInstance()._log.info("server block Skip empty or comment line :" + line);
            continue;
        }
        
        afterValue = getInstance()._cleanValue(afterValue, '#');
        getInstance()._log.info("========= Parsing server block line: key = [" + key + "] line = [" + line + "]");
        if (key == "route") {
            RouteConfig routeConfig;
            routeConfig.route = getInstance()._parseValue(lineStream.str());
            if (afterValue == "{") {
                parseRouteConfig(configFile, &routeConfig);
                serverConfig->routes.push_back(routeConfig);
                getInstance()._log.info(">>>>>>>>>>>>>>>>>>>>> new route config add to server");
            } else {
                std::cerr << "Parsing error !" << std::endl;
                throw std::runtime_error("Malformed route block");
            }
        } else {
            if (key == "listen") {
                serverConfig->listen_port = Utils::stoi<int>(getInstance()._parseValue(lineStream.str()));
            } else if (key == "host") {
                serverConfig->host = getInstance()._parseValue(lineStream.str());
            } else if (key == "server_name") {
                std::istringstream serverNames(line);
                std::string serverName;
                serverNames >> serverName;
                while (serverNames >> serverName && serverName[0] != '#') {
                    serverName = getInstance()._cleanValue(serverName, ';');
                    serverConfig->server_names.push_back(serverName);
                }
            } else if (key == "root") {
                serverConfig->root = getInstance()._parseValue(lineStream.str());
            } else if (key == "client_max_body_size") {
                serverConfig->client_max_body_size = Utils::stoi<int>(getInstance()._parseValue(lineStream.str()));
            } else if (key == "error_page") {
                int errorCode;
                std::string errorPage;
                errorCode = Utils::stoi<int>(value);
                getInstance()._log.info("Server block find error_page: errorCode = " + Utils::to_string(errorCode) + " errorPage = " + afterValue);
                errorPage = getInstance()._cleanValue(afterValue, ';');
                serverConfig->error_pages[errorCode] = errorPage;
            }
            else {
                getInstance()._log.warning("Server block find unknown key: key = " + key + " value = " + value);
            }
        }
        key = "";
    }
}

void ConfigLoader::parseRouteConfig(std::ifstream& configFile, RouteConfig* routeConfig) {
    std::string line;
    std::string key;

    while (key != "}" && getline(configFile, line)) {
        std::istringstream lineStream(line);
        lineStream >> key;
        if (key.empty() || key[0] == '#' || key == "}") {
            getInstance()._log.info("route block Skip empty or comment line :" + line);
            continue;
        }
        getInstance()._log.info(" ====== Parsing route block line: key = " + key + " line = " + line);
        std::string value;
        if (key == "directory") {
            routeConfig->directory = getInstance()._parseValue(lineStream.str());
        } else if (key == "default_file") {
            routeConfig->default_file = getInstance()._parseValue(lineStream.str());
        } else if (key == "allow_methods") {
            std::istringstream methods(line);
            std::string method;
            methods >> method;
            while (methods >> method && method[0] != '#') {
                method = getInstance()._cleanValue(method, ';');
                routeConfig->allow_methods.push_back(method);
            }
        } else if (key == "redirect") {
            routeConfig->redirect = getInstance()._parseValue(lineStream.str());
        } else if (key == "directory_listing") {
            routeConfig->directory_listing = (getInstance()._parseValue(lineStream.str()) == "on");
        } else if (key == "cgi") {
            routeConfig->cgi_handler = getInstance()._parseValue(lineStream.str());
        } else if (key == "accept_upload") {
            routeConfig->upload_path = getInstance()._parseValue(lineStream.str());
        }
        key = "";
    }
}

std::string ConfigLoader::_cleanValue(std::string toClean, char c)
{
    size_t comPos = toClean.find(c);
    if (comPos != std::string::npos) {
        toClean = toClean.substr(0, comPos);
    }
    return toClean;
}

std::string ConfigLoader::_parseValue(std::string toParse)
{
    std::istringstream toParseStream(toParse);
    std::string key;
    std::string value;
    std::string afterValue;
    
    _log.info("Parsing value form :" + toParse);
    toParseStream >> key >> value >> afterValue;
    _log.info("parsed Value :" + value + " afterValue :" + afterValue);
    value = _cleanValue(value, '#');
    afterValue = _cleanValue(afterValue, '#');
    if (key == "route" && afterValue != "{")
        _log.warning("Badly terminated route line in server block: " + toParse + " afterValue = " + afterValue);
    else if (key =="server" && value != "{")
        _log.warning("Badly terminated server line in server block: " + toParse + " afterValue = " + afterValue);
    else
    {
        _log.info("key = " + key + " value = " + value + " afterValue = " + afterValue);
        size_t semicolonPos = value.find(';');
        if (semicolonPos != value.length()) {
            value = value.substr(0, semicolonPos);
        }
        else
            _log.warning("Badly terminated line in server block: " + toParse);
    }
    _log.info("Parsed value : "+ value);
    return value;
}

std::string ConfigLoader::getServerConfigValue(const ServerConfig& config, const std::string& key) const {
    if (key == "listen_port") {
        return Utils::to_string(config.listen_port);
    } else if (key == "host") {
        return config.host;
    } else if (key == "server_name") {
        return config.server_names[0];
    } else if (key == "client_max_body_size") {
        return Utils::to_string(config.client_max_body_size);
    } else {
        _log.warning("Key not found in ServerConfig: " + key);
        return "";
    }
}

const Config& ConfigLoader::getConfig() const {
  return  _instance->_config;
}

void ConfigLoader::printConfig() {
    const Config& config = getInstance()._config;
    for (std::vector<ServerConfig>::const_iterator it = config.servers.begin(); it != config.servers.end(); ++it) {
        const ServerConfig& server = *it;
        std::cout << "====Server on port: " << server.listen_port << std::endl;
        std::cout << "Host: " << server.host << std::endl;
        std::cout << "Server Names: ";
        for (std::vector<std::string>::const_iterator nameIt = server.server_names.begin(); nameIt != server.server_names.end(); ++nameIt) {
            std::cout << *nameIt << " ";
        }
        std::cout << std::endl;
        std::cout << "Root: " << server.root << std::endl;
        std::cout << "Client Max Body Size: " << server.client_max_body_size << std::endl;

        // Afficher les pages d'erreur
        std::cout << "====Error Pages:" << std::endl;
        for (std::map<int, std::string>::const_iterator errorIt = server.error_pages.begin(); errorIt != server.error_pages.end(); ++errorIt) {
            std::cout << " \tError Code: " << errorIt->first << " - Page: " << errorIt->second << std::endl;
        }

        // Afficher les routes
        std::cout << "Routes:" << std::endl;
        for (std::vector<RouteConfig>::const_iterator routeIt = server.routes.begin(); routeIt != server.routes.end(); ++routeIt) {
            const RouteConfig& routeConfig = *routeIt;
            std::cout << "========Route: " << routeConfig.route << std::endl;
            std::cout << "\tDirectory: " << routeConfig.directory << std::endl;
            std::cout << "\tDefault File: " << routeConfig.default_file << std::endl;
            std::cout << "\tRedirect: " << routeConfig.redirect << std::endl;
            std::cout << "\tDirectory Listing: " << (routeConfig.directory_listing ? "on" : "off") << std::endl;
            std::cout << "\tCGI Handler: " << routeConfig.cgi_handler << std::endl;
            std::cout << "\tUpload Path: " << routeConfig.upload_path << std::endl;
            std::cout << "\tAllowed Methods: ";
            for (std::vector<std::string>::const_iterator methodIt = routeConfig.allow_methods.begin(); methodIt != routeConfig.allow_methods.end(); ++methodIt) {
                std::cout << *methodIt << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

ConfigLoader::ConfigLoader() : _log(Logger::getInstance()) {
}

ConfigLoader::~ConfigLoader() {}
