/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:37 by agaley            #+#    #+#             */
/*   Updated: 2024/05/22 20:40:06 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "FileHandler.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include "Config.hpp"
#include "FileManager.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

FileHandler::FileHandler() {}

FileHandler::~FileHandler() {}

HTTPResponse FileHandler::processRequest(const HTTPRequest& request,
                                         ServerConfig&      config) {
  (void)request;
  HTTPResponse response;
  std::string  path = config.root + request.getUrl();
  struct stat  path_stat;
  stat(path.c_str(), &path_stat);

  // Check if the path is a directory
  if (S_ISDIR(path_stat.st_mode)) {
    // Handle directory request
    std::string indexFilePath = path + "/index.html";
    if (FileManager::doesFileExists(indexFilePath)) {
      response.setBody(FileManager::readFile(indexFilePath));
      response.setStatusCode(HTTPResponse::OK);
      response.addHeader("Content-Type", "text/html");
    } else
      response.setStatusCode(HTTPResponse::FORBIDDEN);
  } else if (FileManager::doesFileExists(path)) {
    // Handle file request
    response.setBody(FileManager::readFile(path));
    response.setStatusCode(HTTPResponse::OK);
    // TODO : Detect content type based on actual file type
    response.addHeader("Content-Type", "text/html");
  } else
    response.setStatusCode(HTTPResponse::NOT_FOUND);

  return response;
}
