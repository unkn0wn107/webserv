/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Common.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 11:09:02 by  mchenava         #+#    #+#             */
/*   Updated: 2024/07/02 12:48:06 by  mchenava        ###   ########.fr       */
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

enum ConnectionStatus { READING, EXECUTING, SENDING, CLOSED };

#endif
