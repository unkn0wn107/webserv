/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 03:08:36 by agaley            #+#    #+#             */
/*   Updated: 2024/05/24 03:29:17 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

TrieNode::~TrieNode() {
  delete locationConfig;
  for (std::map<char, TrieNode*>::iterator it = children.begin();
       it != children.end(); ++it) {
    delete it->second;
  }
}

ServerConfig::ServerConfig() : locationTrie(new TrieNode()) {}

// ServerConfig::ServerConfig(const ServerConfig& other) :
//   listen_port(other.listen_port),
//   server_names(other.server_names),
//   root(other.root),
//   error_pages(other.error_pages),
//   client_max_body_size(other.client_max_body_size) {
//   if (other.locationTrie != NULL) {
//     locationTrie = new TrieNode(*(other.locationTrie));
//   } else {
//     locationTrie = NULL;
//   }
// }

// ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
//   if (this != &other) {
//     listen_port = other.listen_port;
//     server_names = other.server_names;
//     root = other.root;
//     error_pages = other.error_pages;
//     client_max_body_size = other.client_max_body_size;

//     delete locationTrie; // Free existing tree
//     if (other.locationTrie != NULL) {
//       locationTrie = new TrieNode(*(other.locationTrie));
//     } else {
//       locationTrie = NULL;
//     }
//   }
//   return *this;
// }
