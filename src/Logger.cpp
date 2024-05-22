/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:32:08 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 17:15:32 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include <algorithm>
#include <ctime>
#include <iostream>
#include <sstream>
#include "Config.hpp"
#include "ConfigLoader.hpp"
#include "Utils.hpp"

Logger* Logger::_instance = NULL;

Logger::Logger() : _progLogFile(new std::ofstream()) {}

Logger& Logger::getInstance() {
  if (_instance == NULL) {
    _instance = new Logger();
  }
  return *_instance;
}

Logger::~Logger() {
  if (_progLogFile->is_open()) {
    _progLogFile->close();
  }
  delete _progLogFile;
}

void Logger::_openLogFile() {
  std::string fileName = _progLogFileName;
  _progLogFile->open(fileName.c_str(), std::ofstream::out | std::ofstream::app);
  if (!_progLogFile->is_open()) {
    std::cerr << "Fatal: Failed to open log file: " + fileName << std::endl;
    return;
  }
}

void Logger::info(const std::string& message) const {
  std::string msg = "[" + _getCurrentTime() + "] INFO: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cout << msg << std::endl;
}

void Logger::warning(const std::string& message) const {
  std::string msg = "[" + _getCurrentTime() + "] WARNING: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cout << msg << std::endl;
}

void Logger::error(const std::string& message) const {
  std::string msg = "[" + _getCurrentTime() + "] ERROR: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cerr << msg << std::endl;
}

std::string Logger::_getCurrentTime() const {
  std::time_t now = std::time(NULL);
  std::tm*    ltm = std::localtime(&now);
  char        buffer[9];  // HH:MM:SS
  std::strftime(buffer, sizeof(buffer), "%H-%M-%S", ltm);
  return std::string(buffer);
}

void Logger::_setFileName() {
  if (_config.log_file.empty())
    _progLogFileName = std::string(LOG_FILE_PATH) + "log_webserv_" +
                       _getCurrentTime() + ".log";
  else
    _progLogFileName = _config.log_file + "_" + _getCurrentTime() + ".log";
}

void Logger::setConfig(const Config& config) {
  getInstance()._config = config;
  _setFileName();
  _openLogFile();
}
