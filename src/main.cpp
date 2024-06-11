/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:02:12 by agaley            #+#    #+#             */
/*   Updated: 2024/06/10 18:53:44 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include "Config.hpp"
#include "ConfigManager.hpp"
#include "Server.hpp"
#include "Common.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2 || argv[1] == NULL)
    Logger::getInstance().error(
        "Bad arguments. usage : ./webserv [config_path]");

  Config config;
  try {
    ConfigManager::loadConfig(argc == 1 ? ConfigManager::DEFAULT_FILE_NAME
                                        : argv[1]);
    Logger::getInstance().info(
        "WebServ running...\n\nCurrent configuration:\n\n");
    ConfigManager::printConfig();
    config = ConfigManager::getInstance().getConfig();
    struct sigaction sigHandler;
    sigHandler.sa_handler = signalHandler;
    sigemptyset(&sigHandler.sa_mask);
    sigHandler.sa_flags = 0;

    sigaction(SIGINT, &sigHandler, NULL);
    sigaction(SIGTERM, &sigHandler, NULL);
    Server server;
    server.start();
  } catch (const std::exception& e) {
    Logger::getInstance().error("Shutdown Error in main.cpp");
    return EXIT_FAILURE;
  }
  Logger::getInstance().info("Shutdown WebServ");
  delete &Logger::getInstance();
  return EXIT_SUCCESS;
}
