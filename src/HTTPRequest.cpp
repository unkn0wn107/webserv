/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:58 by agaley            #+#    #+#             */
/*   Updated: 2024/07/22 18:35:42 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"
#include "Common.hpp"
#include "Utils.hpp"

const std::string HTTPRequest::supportedMethods[4] = {"GET", "HEAD", "DELETE", "POST"};

HTTPRequest::HTTPRequest(std::string rawRequest)
    : _rawRequest(rawRequest), _method(""), _uri(""), _body("") {
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
  _uriComponents = URI::parse(_uri);

  _parseHeaders(requestStream);
  _parseSession();
}

void HTTPRequest::_parseHeaders(std::istringstream& requestStream) {
  std::string line;
  while (std::getline(requestStream, line) && !line.empty()) {
    if (line == "\r" || line.empty())
      break;
    std::size_t pos = line.find(":");
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 2);
      if (!value.empty() && value[value.size() - 1] == '\r')
        value = value.substr(0, value.size() - 1);
      addHeader(key, value);
    }
  }
}

void HTTPRequest::_parseSession() {
  std::string sessionId;
  std::string cookieHeader = getHeader("Cookie");
  std::size_t pos = cookieHeader.find("sessionid=");
  if (pos != std::string::npos) {
    std::size_t endPos = cookieHeader.find(";", pos);
    if (endPos == std::string::npos)
      sessionId = cookieHeader.substr(pos + 10);
    else
      sessionId = cookieHeader.substr(pos + 10, endPos - pos - 10);
  }
  if (sessionId.empty())
    sessionId = Utils::generateSessionId();
  setSessionId(sessionId);
}

void HTTPRequest::setSessionId(const std::string& sessionId) {
  _sessionId = sessionId;
}

std::string HTTPRequest::getSessionId() const {
  return _sessionId;
}

void HTTPRequest::setMethod(const std::string& method) {
  _method = method;
}
void HTTPRequest::setURI(const std::string& uri) {
  _uri = uri;
}

void HTTPRequest::setHeaders(const std::map<std::string, std::string>& headers) {
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
    return -1;
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
