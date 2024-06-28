/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 18:58:00 by agaley            #+#    #+#             */
/*   Updated: 2024/06/28 14:18:45 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

LocationConfig::LocationConfig() {
  client_max_body_size = 1000;
  upload = false;
  cgi = false;
  autoindex = false;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
  location = other.location;
  client_max_body_size = other.client_max_body_size;
  root = other.root;
  index = other.index;
  upload = other.upload;
  delete_ = other.delete_;
  cgi = other.cgi;
  autoindex = other.autoindex;
  error_pages = other.error_pages;
  allowed_methods = other.allowed_methods;
  returnCode = other.returnCode;
  returnUrl = other.returnUrl;
  return *this;
}

LocationConfig::~LocationConfig() {
  error_pages.clear();
  allowed_methods.clear();
}

ListenConfig::ListenConfig() {
  port = 80;
  default_server = false;
  backlog = 10000;
  rcvbuf = 10000;
  sndbuf = 10000;
  ipv6only = false;
}

bool ListenConfig::operator<(const ListenConfig& other) const {
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

bool ListenConfig::operator==(const ListenConfig& other) const {
  return address == other.address && port == other.port &&
         default_server == other.default_server && backlog == other.backlog &&
         rcvbuf == other.rcvbuf && sndbuf == other.sndbuf &&
         ipv6only == other.ipv6only;
}

ServerConfig::ServerConfig() {
  client_max_body_size = 1000;
  upload = false;
  cgi = false;
  autoindex = false;
}

ServerConfig::~ServerConfig() {
  error_pages.clear();
  locations.clear();
}

Config::Config() {
  worker_processes = sysconf(_SC_NPROCESSORS_CONF);
  worker_connections = 2048;
}

Config::~Config() {
  unique_listen_configs.clear();
  servers.clear();
}
