#include "config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

Config::Config(const std::string& configFilePath) : configFilePath(configFilePath) {
}

Config::~Config() {
}

bool Config::loadConfiguration() {
    std::ifstream configFile(configFilePath.c_str());
    if (!configFile.is_open()) {
        return false;
    }

    std::string line;
    while (getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        if (key == "server") {
            parseServerConfig(line);
        } else if (key == "route") {
            parseRouteConfig(line);
        }
    }

    configFile.close();
    return true;
}

ServerConfig Config::getServerConfig(const std::string& host, int port) const {
    for (const auto& server : servers) {
        if (server.host == host && server.port == port) {
            return server;
        }
    }
    throw std::runtime_error("Server configuration not found for the given host and port.");
}

RouteConfig Config::getRouteConfig(const std::string& path) const {
    auto it = routes.find(path);
    if (it != routes.end()) {
        return it->second;
    }
    throw std::runtime_error("Route configuration not found for the given path.");
}

void Config::parseServerConfig(const std::string& line) {
    std::istringstream iss(line);
    std::string key;
    ServerConfig serverConfig;

    while (iss >> key) {
        if (key == "host") {
            iss >> serverConfig.host;
        } else if (key == "port") {
            iss >> serverConfig.port;
        } else if (key == "server_name") {
            std::string name;
            while (iss >> name) {
                serverConfig.server_names.push_back(name);
            }
        } else if (key == "error_page") {
            iss >> serverConfig.default_error_page;
        } else if (key == "body_size_limit") {
            iss >> serverConfig.client_body_size_limit;
        }
    }

    servers.push_back(serverConfig);
}

void Config::parseRouteConfig(const std::string& line) {
    std::istringstream iss(line);
    std::string key;
    std::string path;
    RouteConfig routeConfig;

    iss >> path; // Assume first token after "route" is the path

    while (iss >> key) {
        if (key == "methods") {
            std::string method;
            while (iss >> method) {
                routeConfig.accepted_methods.push_back(method);
            }
        } else if (key == "redirect") {
            iss >> routeConfig.redirection_target;
        } else if (key == "root") {
            iss >> routeConfig.root_directory;
        } else if (key == "list_dir") {
            std::string flag;
            iss >> flag;
            routeConfig.directory_listing_enabled = (flag == "on");
        } else if (key == "default_file") {
            iss >> routeConfig.default_file;
        } else if (key == "cgi") {
            iss >> routeConfig.cgi_path;
        } else if (key == "upload_path") {
            iss >> routeConfig.upload_path;
        }
    }

    routes[path] = routeConfig;
}
