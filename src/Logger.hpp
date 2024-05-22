/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.class.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:21:59 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 17:16:22 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include "Config.hpp"

#define LOG_FILE_PATH "./logs/"
#define LOG_FILE_NAME "webserv"
#define LOG_FILE_EXT ".log"

class Logger {
 public:
  static Logger& getInstance();
  void           setConfig(const Config& config);

  void info(const std::string& message) const;
  void warning(const std::string& message) const;
  void error(const std::string& message) const;

 private:
  Logger();
  ~Logger();

  static Logger* _instance;

  Config         _config;
  std::ofstream* _progLogFile;
  std::string    _progLogFileName;
  std::string    _getCurrentTime() const;

  void _openLogFile();
  void _closeLogFile();
  void _setFileName();
};

#endif /* LOGGER_CLASS_HPP */
