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
  static Logger& getInstance();

  /**
   * Logs a message with a severity level.
   * @param message The message to log.
   * @param level The severity level of the message.
   */
  void log(const std::string& message, int level);

 private:
  Logger();
  Logger(const Logger&);
  Logger& operator=(const Logger&);
};

#endif
