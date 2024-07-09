/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:32:08 by mchenava          #+#    #+#             */
/*   Updated: 2024/07/05 22:36:38 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <ctime>
#include <iostream>
#include <sstream>

#include "Logger.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include "Common.hpp"

Logger* Logger::_instance = NULL;

Logger::Logger() : _progLogFile(new std::ofstream()) {
  pthread_mutex_init(&_mutex, NULL);
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
  pthread_mutex_destroy(&_mutex);
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
#if LOG_LEVEL >= LOG_LEVEL_INFO
  LockGuard lock(_mutex);
  std::string msg = "[" + _getCurrentTime() + "] INFO: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cout << "\033[1;32m" << msg << "\033[0m" << std::endl;  // Green
#else
  (void)message; // Avoid unused parameter warning
#endif
}

void Logger::warning(const std::string& message) const {
#if LOG_LEVEL >= LOG_LEVEL_WARNING
  LockGuard lock(_mutex);
  std::string msg = "[" + _getCurrentTime() + "] WARNING: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cout << "\033[1;33m" << msg << "\033[0m" << std::endl;  // Yellow
#else
  (void)message; // Avoid unused parameter warning
#endif
}

void Logger::error(const std::string& message) const {
#if LOG_LEVEL >= LOG_LEVEL_ERROR
  LockGuard lock(_mutex);
  std::string msg = "[" + _getCurrentTime() + "] ERROR: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cerr << "\033[1;31m" << msg << "\033[0m" << std::endl;  // Red
#else
  (void)message; // Avoid unused parameter warning
#endif
}

void Logger::emerg(const std::string& message) const {
#if LOG_LEVEL >= LOG_LEVEL_ERROR // Assuming EMERG is at least as critical as ERROR
  LockGuard lock(_mutex);
  std::string msg = "[" + _getCurrentTime() + "] EMERG: " + message;
  if (_progLogFile->is_open()) {
    *_progLogFile << msg << std::endl;
  }
  std::cerr << "\033[1;35m" << msg << "\033[0m" << std::endl;  // Magenta
#else
  (void)message; // Avoid unused parameter warning
#endif
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
