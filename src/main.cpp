/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:02:12 by agaley            #+#    #+#             */
/*   Updated: 2024/07/03 01:20:53 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>

#include "Common.hpp"
#include "Config.hpp"
#include "ConfigManager.hpp"
#include "Server.hpp"

int main(int argc, char* argv[]) {
  if (argc > 2) {
    Logger::getInstance().error(
        "Bad arguments. usage : ./webserv [config_path]");
    return EXIT_FAILURE;
  }
  struct sigaction sigHandler;
  sigHandler.sa_handler = signalHandler;
  sigemptyset(&sigHandler.sa_mask);
  sigHandler.sa_flags = 0;

  sigaction(SIGINT, &sigHandler, NULL);
  sigaction(SIGTERM, &sigHandler, NULL);

  srand(static_cast<unsigned int>(time(NULL)));

  Config config;
  try {
    ConfigManager::loadConfig(argc == 1 ? ConfigManager::DEFAULT_FILE_NAME
                                        : argv[1]);
    Logger::getInstance().info(
        "WebServ running...\n\nCurrent configuration:\n\n");
    ConfigManager::printConfig();
    config = ConfigManager::getInstance().getConfig();
  } catch (const std::exception& e) {
    Logger::getInstance().error("Shutdown Error in main.cpp");
    return EXIT_FAILURE;
  }
  Server server;
  Logger::getInstance().info("Starting WebServ");
  server.start();
  Logger::getInstance().info("Shutdown WebServ");
  usleep(SHUTDOWN_DELAY);  // Wait for Logger to finish output logs
  Logger::deleteInstance();
  return EXIT_SUCCESS;
}
