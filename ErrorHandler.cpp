#include "ErrorHandler.hpp"

ErrorHandler::ErrorHandler() {}

void ErrorHandler::exception(const std::exception& ex) {
  std::cerr << "Exception: " << ex.what() << std::endl;
}

void ErrorHandler::log(const std::string& errorMessage) {
  std::cerr << "Error: " << errorMessage << std::endl;
}

void ErrorHandler::fatal(const std::string& errorMessage) {
  std::cerr << "Fatal Error: " << errorMessage << std::endl;
  std::exit(EXIT_FAILURE);
}
