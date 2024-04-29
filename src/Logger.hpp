#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

/**
 * Singleton Logger class for logging messages with different severity levels.
 */
class Logger {
 public:
  /**
   * Returns the singleton instance of Logger.
   */
  static Logger& getInstance() {
    static Logger instance;
    return instance;
  }

  /**
   * Logs a message with a severity level.
   * @param message The message to log.
   * @param level The severity level of the message.
   */
  void log(const std::string& message, int level) {
    switch (level) {
      case 1:
        std::cerr << "[ERROR] " << message << std::endl;
        break;
      case 2:
        std::cerr << "[WARNING] " << message << std::endl;
        break;
      case 3:
        std::cout << "[INFO] " << message << std::endl;
        break;
      default:
        std::cout << "[DEBUG] " << message << std::endl;
        break;
    }
  }

 private:
  Logger() {}
  Logger(const Logger&);
  Logger& operator=(const Logger&);
};

#endif
