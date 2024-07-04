/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:32:08 by mchenava          #+#    #+#             */
/*   Updated: 2024/07/04 03:28:20 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <ctime>
#include <iostream>
#include <sstream>
#include "Config.hpp"

Logger* Logger::_instance = NULL;

Logger::Logger() : _progLogFile(new std::ofstream()) {
}

Logger& Logger::getInstance() {
  if (_instance == NULL) {
    _instance = new Logger();
  }
  return *_instance;
}

void Logger::deleteInstance() {
  if (_instance != NULL) {
    delete _instance;
    _instance = NULL;
  }
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
  std::cout << "\033[1;32m" << msg << "\033[0m" << std::endl; // Green
}

void Logger::warning(const std::string& message) const {
  std::string msg = "[" + _getCurrentTime() + "] WARNING: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cout << "\033[1;33m" << msg << "\033[0m" << std::endl; // Yellow
}

void Logger::error(const std::string& message) const {
  std::string msg = "[" + _getCurrentTime() + "] ERROR: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cerr << "\033[1;31m" << msg << "\033[0m" << std::endl; // Red
}

void Logger::emerg(const std::string& message) const {
  std::string msg = "[" + _getCurrentTime() + "] EMERG: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cerr << "\033[1;35m" << msg << "\033[0m" << std::endl; // Magenta
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

void Logger::setConfig(const Config config) {
  getInstance()._config = config;
  _setFileName();
  // _openLogFile();
}
