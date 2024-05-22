/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 16:53:47 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/11 16:32:17 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <map>
#include <string>
#include <vector>

typedef struct RouteConfig {
  std::string              route;
  std::string              directory;
  std::string              default_file;
  std::vector<std::string> allow_methods;
  std::string              redirect;
  bool                     directory_listing;
  std::string              cgi_handler;
  std::string              upload_path;
} RouteConfig;

typedef struct ServerConfig {
  int                        listen_port;
  std::string                host;
  std::vector<std::string>   server_names;
  std::string                root;
  std::map<int, std::string> error_pages;
  int                        client_max_body_size;
  std::vector<RouteConfig>   routes;
} ServerConfig;

typedef struct Config {
  std::string               log_file;
  std::vector<ServerConfig> servers;
} Config;

#endif  // SERVER_CONFIG_HPP
