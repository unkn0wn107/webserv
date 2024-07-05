/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Common.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 11:09:02 by  mchenava         #+#    #+#             */
/*   Updated: 2024/07/05 22:39:03 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMON_HPP
#define COMMON_HPP

#include <fcntl.h>
#include <string>
#include "Config.hpp"

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3

#define LOG_LEVEL LOG_LEVEL_INFO

int         set_non_blocking(int sockfd);
void        signalHandler(int signum);
std::string generateSessionId(void);

enum ConnectionStatus { READING, EXECUTING, SENDING, ERROR, CLOSED };

enum CGIState {
  NONE,
  INIT,
  CACHE_CHECK,
  REGISTER_SCRIPT_FD,
  RUN_SCRIPT,
  SCRIPT_RUNNING,
  READ_FROM_CGI,
  PROCESS_OUTPUT,
  FINALIZE_RESPONSE,
  ADD_HEADERS,
  DONE,
  CGI_ERROR,
};

#endif
