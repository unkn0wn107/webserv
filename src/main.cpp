/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/28 15:02:12 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 19:52:59 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLoader.hpp"
#include "Server.hpp"

int main(int argc, char* argv[]) {
  if (argc > 2)
    ErrorHandler::fatal("Too many arguments. usage : ./webserv [config_path]");

  std::map<std::string, std::string> config;
  try {
    if (argc == 1) {
      ConfigLoader& config = ConfigLoader::getInstance();
      config.loadConfig("server_config.cfg");
    } else if (argc == 2) {
      ConfigLoader& config = ConfigLoader::getInstance();
      config.loadConfig(argv[1]);
    }
    Server* server = new Server(config);
    server->start();
  } catch (const std::exception& e) {
    ErrorHandler::exception(e);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
