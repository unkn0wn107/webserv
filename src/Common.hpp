/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Common.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 11:09:02 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/17 19:44:02 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMON_HPP
#define COMMON_HPP

#include <fcntl.h>
#include <string>

int         set_non_blocking(int sockfd);
void        signalHandler(int signum);
std::string generateSessionId(void);

#endif
