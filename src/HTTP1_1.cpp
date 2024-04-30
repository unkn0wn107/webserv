/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP1_1.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:48 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 19:59:16 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP1_1.hpp"

HTTP1_1::HTTP1_1() {}

HTTP1_1::~HTTP1_1() {}

HTTPRequest HTTP1_1::parseRequest(const std::string& requestData) {
  HTTPRequest        request;
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

  if (request.getMethod() == "GET")
    response.setBody("Received a GET request for " + request.getUrl());
  else if (request.getMethod() == "HEAD")
    response.setBody("Received a HEAD request with body: " + request.getBody());
  else if (request.getMethod() == "DELETE")
    response.setBody("Received a DELETE request with body: " +
                     request.getBody());
  else if (request.getMethod() == "POST")
    response.setBody("Received a POST request with body: " + request.getBody());
  else
    response.setStatusCode(HTTPResponse::METHOD_NOT_ALLOWED);

  return response;
}
