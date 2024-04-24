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

#include "Logger.class.hpp"
#include <iostream>
#include <ctime>
#include <algorithm>

Logger::Logger() : _logFile(new std::ofstream), _logFileName(LOG_FILE_NAME) {
    _logFileName = _generateLogFileName();
    _logFile->open(_logFileName.c_str(), std::ofstream::out | std::ofstream::app);
	if (!_logFile->is_open()) {
		std::cerr << "Failed to open log file: " << _logFileName << std::endl;
	}
}

Logger::~Logger() {
    if (_logFile->is_open()) {
        _logFile->close();
    }
    delete _logFile;
}

void Logger::info(const std::string& message) const {
	std::string msg = "[" + _getCurrentTime() + "] INFO: " + message;
    if (_logFile->is_open()) {
        *_logFile << msg << std::endl;
    }
	std::cout << msg << std::endl;
}

void Logger::warning(const std::string& message) const {
	std::string msg = "[" + _getCurrentTime() + "] WARNING: " + message;
    if (_logFile->is_open()) {
        *_logFile << msg << std::endl;
    }
	std::cout << msg << std::endl;
}

void Logger::error(const std::string& message) const {
	std::string msg = "[" + _getCurrentTime() + "] ERROR: " + message;
    if (_logFile->is_open()) {
        *_logFile << msg << std::endl;
    }
	std::cerr << msg << std::endl;
}

void Logger::setLogFileName(const std::string& logFileName) {
    if (_logFile->is_open()) {
        _logFile->close();
    }
    _logFileName = logFileName;
    std::string file = std::string(LOG_FILE_PATH) + _logFileName;
    _logFile->open(file.c_str(), std::ofstream::out | std::ofstream::app);
}

std::string Logger::_getCurrentTime() const {
    std::time_t now = std::time(NULL);
    std::tm *ltm = std::localtime(&now);
    char buffer[9]; // HH:MM:SS
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", ltm);
    return std::string(buffer);
}

std::string Logger::_generateLogFileName() {
    std::string timeCode = _getCurrentTime();
    std::replace(timeCode.begin(), timeCode.end(), ':', '-');
    return std::string(LOG_FILE_PATH) + _logFileName + timeCode + ".log";
}
