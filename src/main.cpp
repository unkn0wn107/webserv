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

int main(int argc, char* argv[]) {
  if (argc > 2)
    ErrorHandler::fatal("Too many arguments. usage : ./webserv [config_path]");

  std::map<std::string, std::string> config;
  try {
    ConfigLoader& config = ConfigLoader::getInstance();
    if (argc == 1)
      config.loadConfig(ConfigLoader::DEFAULT_FILE_NAME);
    else if (argc == 2)
      config.loadConfig(argv[1]);
    Server* server = new Server(config.getConfig());
    server->start();
  } catch (const std::exception& e) {
    ErrorHandler::exception(e);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
