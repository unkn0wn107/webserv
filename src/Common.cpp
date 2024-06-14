/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Common.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/24 11:06:33 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/14 13:38:25 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <string.h>
#include "Logger.hpp"
#include "Utils.hpp"
#include "Server.hpp"

int set_non_blocking(int sockfd) {
  int flags, s;
  flags = fcntl(sockfd, F_GETFL);
  if (flags == -1) {
    Logger::getInstance().error(std::string("SET_NON_BLOCKING: fcntl get: ") +
                                strerror(errno));
    return -1;
  }
  flags |= O_NONBLOCK;
  s = fcntl(sockfd, F_SETFL, flags);
  if (s == -1) {
    Logger::getInstance().error(std::string("SET_NON_BLOCKING: fcntl set: ") +
                                strerror(errno));
    return -1;
  }
  return 0;
}


void  signalHandler(int signum) {
  Logger::getInstance().info("Signal received: " + Utils::to_string(signum));
  Server::getInstance().stop(signum);
}


std::string generateSessionId() {
  std::string sessionId;
  std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  for (int i = 0; i < 32; i++) {
    sessionId += chars[rand() % chars.size()];
  }
  return sessionId;
}
