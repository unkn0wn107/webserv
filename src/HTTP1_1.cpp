/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP1_1.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:48 by agaley            #+#    #+#             */
/*   Updated: 2024/05/29 09:57:02 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP1_1.hpp"
#include <algorithm>
#include "Config.hpp"
#include "Logger.hpp"

std::vector<std::string> HTTP1_1::_validMethods;

HTTP1_1::HTTP1_1(ServerConfig& config) : HTTPProtocol(config) {
  _validMethods.push_back("GET");
  _validMethods.push_back("HEAD");
  _validMethods.push_back("DELETE");
  _validMethods.push_back("POST");
}

HTTP1_1::~HTTP1_1() {}

HTTPRequest HTTP1_1::parseRequest(const std::string& requestData) {
  std::istringstream stream(requestData);
  std::string        line;
  std::getline(stream, line);

  // Parse the request line
  std::istringstream requestLine(line);
  std::string        method;
  std::string        url;
  std::string        version;
  requestLine >> method >> url >> version;

  request.setMethod(method);
  request.setUrl(url);

  // Parse headers
  while (std::getline(stream, line) && line != "\r") {
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    if (line.empty())
      continue;
    std::string headerName = line.substr(0, line.find(":"));
    std::string headerValue = line.substr(line.find(":") + 2);
    request.addHeader(headerName, headerValue);
  }

  // Parse body if any
  std::string body;
  if (request.getHeader("Content-Length") != "") {
    size_t contentLength =
        Utils::stoi<size_t>(request.getHeader("Content-Length"));
    char* buffer = new char[contentLength + 1];
    stream.read(buffer, contentLength);
    buffer[contentLength] = '\0';
    body = std::string(buffer, contentLength);
    delete[] buffer;
  }
  request.setBody(body);

  return request;
}

HTTPResponse HTTP1_1::processRequest(const HTTPRequest& request) {
  HTTPResponse response = HTTPResponse("HTTP/1.1");
  if (std::find(_validMethods.begin(), _validMethods.end(),
                request.getMethod()) != _validMethods.end()) {
    response.setBody("Received a " + request.getMethod() + " request for " +
                     request.getUrl());
    _log.info("(" + _config.server_names[0] + ") Received a " +
              request.getMethod() + " request for " + request.getUrl());
  } else
    response.setStatusCode(HTTPResponse::METHOD_NOT_ALLOWED);
  return response;
}
