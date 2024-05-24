/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:21:59 by mchenava          #+#    #+#             */
/*   Updated: 2024/05/24 02:04:28 by agaley           ###   ########lyon.fr   */
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
  static Logger&    getInstance();
  void              setConfig(const Config& config);
  const std::string getFileName() const;

  void info(const std::string& message) const;
  void warning(const std::string& message) const;
  void error(const std::string& message) const;
  void emerg(const std::string& message) const;

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
