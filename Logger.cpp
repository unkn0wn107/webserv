#include "Logger.hpp"
#include <ctime>
#include <fstream>
#include <iostream>

Logger* Logger::instance = nullptr;

Logger::Logger() {
  // Constructor for Logger, private as per Singleton pattern
}

Logger* Logger::getInstance() {
  if (instance == nullptr) {
    instance = new Logger();
  }
  return instance;
}

void Logger::log(const std::string& message, int level) {
  std::ofstream logFile;
  logFile.open("server.log", std::ios_base::app);  // Append mode

  if (!logFile.is_open()) {
    std::cerr << "Failed to open log file." << std::endl;
    return;
  }

  // Get current time and format it
  std::time_t now = std::time(nullptr);
  char        buf[100];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

  // Write log entry
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

void Logger::release() {
  if (instance != nullptr) {
    delete instance;
    instance = nullptr;
  }
}
