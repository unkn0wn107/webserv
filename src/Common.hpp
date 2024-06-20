/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Common.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 11:09:02 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/20 17:29:09 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMON_HPP
#define COMMON_HPP

#include <fcntl.h>
#include <string>
#include "Config.hpp"

int         set_non_blocking(int sockfd);
void        signalHandler(int signum);
std::string generateSessionId(void);

typedef struct s_listen_socket {
    int socket;
    ListenConfig config;
	bool operator<(const s_listen_socket& rhs) const {
        return socket < rhs.socket;
    }
	bool isIn(int fd) const {
		return socket == fd;
	}
} t_listen_socket;

bool isInSet(int fd, std::set<t_listen_socket>& listenSockets);


#endif
