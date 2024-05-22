/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 16:53:47 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/21 13:59:18 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <set>

typedef struct RouteConfig {
    std::string route;
    std::string directory;
    std::string default_file;
    std::vector<std::string> allow_methods;
    std::string redirect;
    bool directory_listing;
    std::string cgi_handler;
    std::string upload_path;
} RouteConfig;

typedef struct ServerConfig {
    bool                        isDefault;
    int                         listen_port;
    std::string                 host;
    std::vector<std::string>    server_names;
    std::string                 root;
    std::map<int, std::string>  error_pages;
    int                         client_max_body_size;
    std::vector<RouteConfig>    routes;
    ServerConfig() : isDefault(false) {}
} ServerConfig;

typedef struct Config {
    std::string log_file;
    std::vector<ServerConfig> servers;
} Config;

#endif // SERVER_CONFIG_HPP
