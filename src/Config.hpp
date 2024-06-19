/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 16:53:47 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/18 17:16:35 by  mchenava        ###   ########.fr       */
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
  unsigned int               client_max_body_size;
  std::string                root;
  std::string                index;
  bool                       upload;
  bool                       delete_;
  bool                       cgi;
  bool                       autoindex;
  std::map<int, std::string> error_pages;

  // Specific to location
  std::set<std::string>    allowed_methods;
  int                      returnCode;
  std::string              returnUrl;

  LocationConfig() {
    client_max_body_size = 1000;
    upload = false;
    cgi = false;
    autoindex = false;
  }
} LocationConfig;

typedef struct ListenConfig {
  std::string address;
  int         port;
  bool        default_server;
  int         backlog;
  int         rcvbuf;
  int         sndbuf;
  bool        ipv6only;

  bool operator<(const ListenConfig& other) const {
    if (address < other.address)
      return true;
    if (address > other.address)
      return false;
    if (port < other.port)
      return true;
    if (port > other.port)
      return false;
    if (default_server < other.default_server)
      return true;
    if (default_server > other.default_server)
      return false;
    if (rcvbuf < other.rcvbuf)
      return true;
    if (rcvbuf > other.rcvbuf)
      return false;
    if (sndbuf < other.sndbuf)
      return true;
    if (sndbuf > other.sndbuf)
      return false;
    return ipv6only < other.ipv6only;
  }
  bool operator==(const ListenConfig& other) const {
    return address == other.address && port == other.port &&
           default_server == other.default_server && backlog == other.backlog &&
           rcvbuf == other.rcvbuf && sndbuf == other.sndbuf &&
           ipv6only == other.ipv6only;
  }
  ListenConfig() {
    port = 80;
    default_server = false;
    backlog = 1000;
    rcvbuf = 1000;
    sndbuf = 1000;
    ipv6only = false;
  }
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

  ServerConfig() {
    client_max_body_size = 1000;
    upload = false;
    cgi = false;
    autoindex = false;
  }
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
