/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:15 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:12:18 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include <ctime>
#include <fstream>
#include <iostream>

Logger::Logger() {}

Logger::~Logger() {}

Logger& Logger::get() {
  static Logger instance;
  return instance;
}

void Logger::log(const std::string& message, logLevel level) {
  std::ofstream logFile;
  logFile.open("server.log", std::ios_base::app);

  if (!logFile.is_open()) {
    std::cerr << "Failed to open log file." << std::endl;
    return;
  }

  std::time_t now = std::time(NULL);
  char        buf[100];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

  logFile << "[" << buf << "] ";

  switch (level) {
    case Logger::INFO:
      logFile << "INFO: ";
      break;
    case Logger::WARNING:
      logFile << "WARNING: ";
      break;
    case Logger::ERROR:
      logFile << "ERROR: ";
      break;
    default:
      logFile << "UNKNOWN: ";
      break;
  }

  logFile << message << std::endl;

  logFile.close();
}
