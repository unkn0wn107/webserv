#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse() : _statusCode(HTTPResponse::OK) {
  _protocol = "HTTP/1.1";
}

HTTPResponse::HTTPResponse(const std::string& protocol)
    : _statusCode(HTTPResponse::OK) {
  _protocol = protocol;
}

void HTTPResponse::setStatusCode(int code) {
  _statusCode = code;
}

void HTTPResponse::setHeaders(const std::map<std::string, std::string>& hdrs) {
  _headers = hdrs;
}

void HTTPResponse::addHeader(const std::string& key, const std::string& value) {
  _headers[key] = value;
}

void HTTPResponse::setBody(const std::string& body) {
  _body = body;
}

int HTTPResponse::getStatusCode() const {
  return _statusCode;
}

std::map<std::string, std::string> HTTPResponse::getHeaders() const {
  return _headers;
}

std::string HTTPResponse::getBody() const {
  return _body;
}

std::string HTTPResponse::getStatusMessage(int code) const {
  std::map<int, std::string>::const_iterator it = statusCodeMessages.find(code);
  if (it != statusCodeMessages.end())
    return it->second;
  else
    return "Unknown";
}

std::string HTTPResponse::generate() const {
  std::string response;
  response += "HTTP/1.1 " + std::to_string(_statusCode) + " " +
              getStatusMessage(_statusCode) + "\r\n";

  for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
       it != _headers.end(); ++it) {
    response += it->first + ": " + it->second + "\r\n";
  }

  response += "Content-Type: text/plain\r\n";

  if (_body.length() > 0) {
    response += "Content-Length: " + std::to_string(_body.length()) + "\r\n";
    response += "\r\n" + _body;
  }

  return response;
}
