/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:02:12 by agaley            #+#    #+#             */
/*   Updated: 2024/05/22 20:10:44 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "ConfigLoader.hpp"
#include "Server.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2 || argv[1] == NULL)
    ErrorHandler::fatal("Bad arguments. usage : ./webserv [config_path]");

  Config config;
  try {
    ConfigLoader::loadConfig(argc == 1 ? ConfigLoader::DEFAULT_FILE_NAME
                                       : argv[1]);
    Logger::getInstance().info(
        "WebServ running...\n\nCurrent configuration:\n\n");
    ConfigLoader::printConfig();
    config = ConfigLoader::getInstance().getConfig();
    for (std::vector<ServerConfig>::iterator it = config.servers.begin();
         it != config.servers.end(); ++it) {
      Server server(*it);
      server.run();
    }
  } catch (const std::exception& e) {
    ErrorHandler::exception(e);
    Logger::getInstance().error("Shutdown Error in main.cpp");
    return EXIT_FAILURE;
  }
  Logger::getInstance().info("Shutdown WebServ");
  return EXIT_SUCCESS;
}
