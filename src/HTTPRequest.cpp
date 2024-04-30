#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest() : _method(""), _url(""), _body("") {}

void HTTPRequest::parse(const std::string& rawRequest) {
  std::istringstream requestStream(rawRequest);
  std::string        line;
  std::getline(requestStream, line);
  std::istringstream lineStream(line);
  lineStream >> _method >> _url;

  while (std::getline(requestStream, line) && line != "\r") {
    auto colonPos = line.find(':');
    if (colonPos != std::string::npos) {
      std::string key = line.substr(0, colonPos);
      std::string value = line.substr(colonPos + 2);  // Skip ": "
      _headers[key] = value;
    }
  }

  // Read the body if any
  _body = std::string(std::istreambuf_iterator<char>(requestStream), {});
}

std::string HTTPRequest::getMethod() const {
  return _method;
}

std::string HTTPRequest::getUrl() const {
  return _url;
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
