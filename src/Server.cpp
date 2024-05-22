/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:34:02 by agaley            #+#    #+#             */
/*   Updated: 2024/05/07 09:16:32 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(Config& config):
	_config(config),
	_log(Logger::getInstance())
{
	_setupServerSockets(_config.servers);
}

Server::~Server() {

}

void Server::_setupServerSockets(const std::vector<ServerConfig>& servers) {
    for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        const ServerConfig& server = *it;
        for (std::vector<ListenConfig>::const_iterator it2 = server.listen.begin(); it2 != server.listen.end(); ++it2) {
            const ListenConfig& listenConfig = *it2;
            if (_uniqueConfigs.insert(listenConfig).second) {  // Vérifie si l'insertion est réussie
                int sock = socket(AF_INET6, SOCK_STREAM, 0);
                if (sock < 0) {
                    _log.error("(" + server.server_names[0] + ") Failed to create socket");
                    continue;
                }

                struct sockaddr_in6 address;
                memset(&address, 0, sizeof(address));
                address.sin6_family = AF_INET6;
                address.sin6_port = htons(listenConfig.port);

                if (listenConfig.address.empty() || listenConfig.address == "*") {
                    address.sin6_addr = in6addr_any;
                } else {
                    if (inet_pton(AF_INET6, listenConfig.address.c_str(), &address.sin6_addr) <= 0) {
                        _log.error("(" + server.server_names[0] + ") Invalid address");
                        close(sock);
                        continue;
                    }
                }

                int opt = 1;
                if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
                    _log.error("(" + server.server_names[0] + ") Failed to set socket options");
                    close(sock);
                    continue;
                }

                if (listenConfig.ipv6only) {
                    int ipv6only = 1;
                    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6only, sizeof(ipv6only)) < 0) {
                        _log.error("(" + server.server_names[0] + ") Failed to set IPV6_V6ONLY");
                        close(sock);
                        continue;
                    }
                }

                if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
                    _log.error("(" + server.server_names[0] + ") Failed to bind socket");
                    close(sock);
                    continue;
                }

                if (listen(sock, listenConfig.backlog) < 0) {
                    _log.error("(" + server.server_names[0] + ") Failed to listen on socket");
                    close(sock);
                    continue;
                }

                _listenSockets[listenConfig] = sock;
            }
        }
    }
}
