#include <iostream>
#include <string>
#include "Server.hpp"

int main(int argc, char* argv[]) {
  if (argc > 2)
    ErrorHandler::fatal("Too many arguments. usage : ./webserv [config_path]");

  try {
    if (argc == 1)
      config = new ConfigLoader().getInstance(null);
    else if (argc == 2)
      config = new ConfigLoader().getInstance(argv[1]);
    Server server = new Server(config);
  } catch (const std::exception& e) {
    ErrorHandler::exception(e);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
