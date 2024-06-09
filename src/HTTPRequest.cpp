/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:58 by agaley            #+#    #+#             */
/*   Updated: 2024/06/09 03:12:07 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"
#include "Utils.hpp"

HTTPRequest::HTTPRequest(std::string rawRequest /*, size_t readn*/)
    : _rawRequest(rawRequest),
      /*_readn(readn),*/ _method(""),
      _uri(""),
      _body("") {
  parseRequest();
}

HTTPRequest::~HTTPRequest() {}

void HTTPRequest::parseRequest() {
  std::istringstream requestStream(_rawRequest);
  std::string        line;
  std::getline(requestStream, line);
  std::istringstream lineStream(line);
  std::string        rawUri;

  lineStream >> _method >> rawUri >> _protocol;

  _uri = URI::decode(rawUri);
  _uriComponents = URI::parse(rawUri);

  std::cout << "URI: " << _uri << std::endl;
  std::cout << "Method: " << _method << std::endl;
  std::cout << "Protocol: " << _protocol << std::endl;
  std::cout << "URI Components:" << std::endl;
  std::cout << "  Extension: " << _uriComponents.extension << std::endl;
  std::cout << "  Script Name: " << _uriComponents.scriptName << std::endl;
  std::cout << "  Path Info: " << _uriComponents.pathInfo << std::endl;
  std::cout << "  Query String: " << _uriComponents.queryString << std::endl;

  while (std::getline(requestStream, line) && !line.empty()) {
    std::size_t pos = line.find(":");
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 2);
      if (!value.empty() && value[value.size() - 1] == '\r')
        value = value.substr(0, value.size() - 1);
      addHeader(key, value);
    }
  }

  if (_method == "POST" || _method == "PUT") {
    std::stringstream bodyStream;
    bodyStream << requestStream.rdbuf();
    setBody(bodyStream.str());
  }
}

void HTTPRequest::setConfig(LocationConfig* config) {
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

int HTTPRequest::getContentLength() const {
  std::string contentLength = getHeader("Content-Length");
  if (contentLength.empty()) {
    return 0;
  }
  return Utils::stoi<int>(contentLength);
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

LocationConfig* HTTPRequest::getConfig() const {
  return _config;
}

std::string HTTPRequest::getMethod() const {
  return _method;
}

std::string HTTPRequest::getURI() const {
  return _uri;
}

URI::Components HTTPRequest::getURIComponents() const {
  return _uriComponents;
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
