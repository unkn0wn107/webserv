#include "ErrorHandler.hpp"
#include <cstdlib>
#include <iostream>

ErrorHandler::ErrorHandler() {
  // Constructor implementation can be empty if no initialization is needed
}

void ErrorHandler::handleError(const std::exception& ex) {
  // Log the error message to standard error stream
  std::cerr << "Error: " << ex.what() << std::endl;

  // Optionally, perform additional logging or error handling here
  // For example, you might want to log the error to a file or send a
  // notification

  // Since the program should not crash, we handle the error gracefully
  // Depending on the severity, you might want to exit with a non-zero status
  // However, for a server, it might be better to keep running unless it's a
  // critical error
}

void ErrorHandler::handleError(const std::string& errorMessage) {
  // Log the error message to standard error stream
  std::cerr << "Error: " << errorMessage << std::endl;

  // Similar to the exception handling, handle the error without crashing the
  // server Additional error handling strategies can be implemented here
}

void ErrorHandler::handleFatalError(const std::string& errorMessage) {
  // Log the fatal error message to standard error stream
  std::cerr << "Fatal Error: " << errorMessage << std::endl;

  // Since this is a fatal error, we might decide to exit the program
  // Exiting might be necessary if the error leaves the server in an unstable
  // state
  std::exit(EXIT_FAILURE);
}
