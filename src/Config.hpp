/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/09 16:53:47 by  mchenava         #+#    #+#             */
/*   Updated: 2024/05/22 19:45:20 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <unistd.h>
#include <map>
#include <set>
#include <string>
#include <vector>

typedef struct LocationConfig {
  std::string              location;
  std::string              root;
  std::string              index;
  std::vector<std::string> allowed_methods;
  int                      returnCode;
  std::string              returnUrl;
  std::string              cgi_handler;
  bool                     autoindex;
  std::string              upload_path;
} LocactionConfig;

struct TrieNode {
  std::map<char, TrieNode*> children;
  LocationConfig*           locationConfig;  // URL-based location Trie
  ~TrieNode();                               // Memory freeing
};

typedef struct ServerConfig {
  int                        listen_port;
  std::vector<std::string>   server_names;
  std::string                root;
  std::map<int, std::string> error_pages;
  int                        client_max_body_size;
  TrieNode*                  locationTrie;  // Root node of location trie

  ServerConfig();  // Struct with trie root init
  // ServerConfig(const ServerConfig& other);
  // ServerConfig& operator=(const ServerConfig& other);
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
