/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 16:53:47 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/22 19:45:20 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <map>
#include <string>
#include <vector>

typedef struct LocationConfig {
  std::string              location;
  std::string              root;
  std::string              index;
  std::vector<std::string> allow_methods;
  int                      returnCode;
  std::string              returnUrl;
  std::string              cgi_handler;
  bool                     autoindex;
  std::string              upload_path;
} RouteConfig;

typedef struct ServerConfig {
  int                         listen_port;
  std::vector<std::string>    server_names;
  std::string                 root;
  std::map<int, std::string>  error_pages;
  int                         client_max_body_size;
  std::vector<LocationConfig> locations;
} ServerConfig;

typedef struct Config {
  std::string               log_file;
  std::vector<ServerConfig> servers;
} Config;

#endif  // SERVER_CONFIG_HPP
