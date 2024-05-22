/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 16:53:47 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/22 18:46:12 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <map>
#include <string>
#include <vector>
#include <set>

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

typedef struct ListenConfig {
	std::string				address;
	int						port;
	bool					default_server;
	int						backlog;
	int						rcvbuf;
	int						sndbuf;
	bool					ipv6only;

	bool operator<(const ListenConfig& other) const {
       if (address < other.address) return true;
       if (address > other.address) return false;
       if (port < other.port) return true;
       if (port > other.port) return false;
       if (default_server < other.default_server) return true;
       if (default_server > other.default_server) return false;
       if (rcvbuf < other.rcvbuf) return true;
       if (rcvbuf > other.rcvbuf) return false;
       if (sndbuf < other.sndbuf) return true;
       if (sndbuf > other.sndbuf) return false;
       return ipv6only < other.ipv6only;
   }
} ListenConfig;

typedef struct ServerConfig {
  std::vector<ListenConfig>		listen;
  std::vector<std::string>		server_names;
  std::string					root;
  std::map<int, std::string>	error_pages;
  int							client_max_body_size;
  std::vector<RouteConfig>		routes;
} ServerConfig;

typedef struct Config {
	int							worker_processes;
	int							worker_connections;

	std::set<ListenConfig>		unique_listen_configs;
	std::string					log_file;
	std::vector<ServerConfig>	servers;
} Config;

#endif  // SERVER_CONFIG_HPP
