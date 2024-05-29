/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 16:53:47 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/29 17:39:30 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_H
#define CONFIG_H

#include <unistd.h>
#include <map>
#include <set>
#include <string>
#include <vector>

typedef struct LocationConfig {
  std::string location;

  // Also in server
  int                        client_max_body_size;
  std::string                root;
  std::string                index;
  std::string                upload_path;
  std::string                cgi_handler;
  bool                       autoindex;
  std::map<int, std::string> error_pages;

  // Specific to location
  std::vector<std::string> allowed_methods;
  int                      returnCode;
  std::string              returnUrl;
} LocationConfig;

typedef struct ServerConfig {
  int                      listen_port;
  std::vector<std::string> server_names;

  // Also in location
  int                        client_max_body_size;
  std::string                root;
  std::string                index;
  std::string                upload_path;
  std::string                cgi_handler;
  bool                       autoindex;
  std::map<int, std::string> error_pages;

  std::map<std::string, LocationConfig*> locations;
} ServerConfig;

typedef struct Config {
  int worker_processes;
  int worker_connections;

  std::set<ListenConfig>    unique_listen_configs;
  std::string               log_file;
  std::vector<ServerConfig> servers;
  Config() {
    worker_processes =
        sysconf(_SC_NPROCESSORS_CONF);  // Initialisation dans le constructeur
    worker_connections = 1000;          // Valeur fixe initialisée ici
  }
} Config;

#endif
