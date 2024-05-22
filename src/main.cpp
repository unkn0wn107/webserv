/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:02:12 by agaley            #+#    #+#             */
/*   Updated: 2024/05/04 01:32:21 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLoader.hpp"
#include "Server.hpp"
#include "Config.hpp"
#include "PortListener.hpp"

int main(int argc, char* argv[]) {
  if (argc != 2 || argv[1] == NULL)
    ErrorHandler::fatal("Bad arguments. usage : ./webserv [config_path]");

  Config config;
  std::vector<PortListener *> listeners;
  try {
    ConfigLoader::loadConfig(argc == 1 ? ConfigLoader::DEFAULT_FILE_NAME : argv[1]);
    Logger::getInstance().info("WebServ running...\n\nCurrent configuration:\n\n");
    ConfigLoader::printConfig();
    listeners = ConfigLoader::creatListeners();
    for (std::vector<PortListener *>::iterator listener = listeners.begin(); listener != listeners.end(); ++listener)
    {
      (*listener)->startListening();
    }
    for (std::vector<PortListener *>::iterator listener = listeners.begin(); listener != listeners.end(); ++listener)
    {
      (*listener)->stopListening();
    }
  } catch (const std::exception& e) {
    ErrorHandler::exception(e);
    Logger::getInstance().error("Shutdown Error in main.cpp");
    return EXIT_FAILURE;
  }
  // while (true)
  // {}
  Logger::getInstance().info("Shutdown WebServ");
  return EXIT_SUCCESS;
}
