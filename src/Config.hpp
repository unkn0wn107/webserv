/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 16:53:47 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/28 08:57:15 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_H
#define CONFIG_H

#include <unistd.h>
#include <map>
#include <set>
#include <string>
#include <vector>


#define BUFFER_SIZE 16384

typedef struct LocationConfig {
  std::string location;

  // Also in server
  unsigned int               client_max_body_size;
  std::string                root;
  std::string                index;
  bool                       upload;
  bool                       delete_;
  bool                       cgi;
  bool                       autoindex;
  std::map<int, std::string> error_pages;

  // Specific to location
  std::set<std::string> allowed_methods;
  int                   returnCode;
  std::string           returnUrl;

  LocationConfig();
  ~LocationConfig();
} LocationConfig;

typedef struct ListenConfig {
  std::string address;
  int         port;
  bool        default_server;
  int         backlog;
  int         rcvbuf;
  int         sndbuf;
  bool        ipv6only;

  bool operator<(const ListenConfig& other) const;
  bool operator==(const ListenConfig& other) const;

  ListenConfig();
} ListenConfig;

typedef struct ServerConfig {
  std::vector<ListenConfig> listen;
  std::vector<std::string>  server_names;

  // Also in location
  int                        client_max_body_size;
  std::string                root;
  std::string                index;
  bool                       upload;
  bool                       delete_;
  bool                       cgi;
  bool                       autoindex;
  std::map<int, std::string> error_pages;

  std::map<std::string, LocationConfig> locations;

  ServerConfig();
  ~ServerConfig();
} ServerConfig;

typedef struct Config {
  int worker_processes;
  int worker_connections;

  std::set<ListenConfig>    unique_listen_configs;
  std::string               log_file;
  std::vector<ServerConfig> servers;

  Config();
  ~Config();
} Config;

#endif
