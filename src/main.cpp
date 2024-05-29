/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:02:12 by agaley            #+#    #+#             */
/*   Updated: 2024/05/29 18:21:26 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include "Config.hpp"
#include "ConfigManager.hpp"
#include "Server.hpp"

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
    Server server;
  } catch (const std::exception& e) {
    Logger::getInstance().error("Shutdown Error in main.cpp");
    return EXIT_FAILURE;
  }
  Logger::getInstance().info("Shutdown WebServ");
  return EXIT_SUCCESS;
}
