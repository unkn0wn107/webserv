/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:58 by agaley            #+#    #+#             */
/*   Updated: 2024/05/29 16:10:00 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"
#include <iostream>

HTTPRequest::HTTPRequest(std::string rawRequest, size_t readn)
    : _rawRequest(rawRequest), _readn(readn), _method(""), _uri(""), _body("") {
  parseRequest();
}

HTTPRequest::~HTTPRequest() {}

void HTTPRequest::parseRequest() {
  std::istringstream requestStream(_rawRequest);
  std::string        line;
  std::getline(requestStream, line);
  std::istringstream lineStream(line);
  lineStream >> _method >> _uri >> _protocol;
  while (std::getline(requestStream, line) && line != "\r") {
    std::size_t pos = line.find(":");
    if (pos != std::string::npos) {
      addHeader(line.substr(0, pos), line.substr(pos + 2));
    }
  }
  if (_method == "POST" || _method == "PUT") {
    std::stringstream bodyStream;
    bodyStream << requestStream.rdbuf();
    setBody(bodyStream.str());
  }
}

void HTTPRequest::configure(ServerConfig& config) {
  _config = config;
}

void HTTPRequest::setMethod(const std::string& method) {
  _method = method;
}
void HTTPRequest::setURI(const std::string& uri) {
  _uri = uri;
}

void HTTPRequest::setHeaders(
    const std::map<std::string, std::string>& headers) {
  _headers = headers;
}

void HTTPRequest::addHeader(const std::string& key, const std::string& value) {
  _headers[key] = value;
}

void HTTPRequest::setBody(const std::string& body) {
  _body = body;
}

std::string HTTPRequest::getProtocol() const {
  return _protocol;
}

std::string HTTPRequest::getHost() const {
  return getHeader("Host");
}

std::string HTTPRequest::getRawRequest() const {
  return _rawRequest;
}

std::string HTTPRequest::getMethod() const {
  return _method;
}

std::string HTTPRequest::getURI() const {
  return _uri;
}

std::map<std::string, std::string> HTTPRequest::getHeaders() const {
  return _headers;
}

std::string HTTPRequest::getBody() const {
  return _body;
}

std::string HTTPRequest::getHeader(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator it = _headers.find(key);
  if (it != _headers.end()) {
    return it->second;
  }
  return "";
}
