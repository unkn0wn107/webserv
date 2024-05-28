/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:02:12 by agaley            #+#    #+#             */
/*   Updated: 2024/05/24 16:44:51 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "ConfigLoader.hpp"
#include "Server.hpp"
#include <cstdlib>

int main(int argc, char* argv[]) {
  if (argc != 2 || argv[1] == NULL)
    Logger::getInstance().error("Bad arguments. usage : ./webserv [config_path]");

  Config config;
  try {
    ConfigLoader::loadConfig(argc == 1 ? ConfigLoader::DEFAULT_FILE_NAME : argv[1]);
    Logger::getInstance().info(
        "WebServ running...\n\nCurrent configuration:\n\n");
    ConfigLoader::printConfig();
    config = ConfigLoader::getInstance().getConfig();
    Server server;
  } catch (const std::exception& e) {
    Logger::getInstance().error("Shutdown Error in main.cpp");
    return EXIT_FAILURE;
  }
  Logger::getInstance().info("Shutdown WebServ");
  return EXIT_SUCCESS;
}
