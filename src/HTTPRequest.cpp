#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest() : method(""), url(""), body("") {}

void HTTPRequest::setMethod(const std::string& method) {
  this->method = method;
}

void HTTPRequest::setUrl(const std::string& url) {
  this->url = url;
}

void HTTPRequest::setHeaders(
    const std::map<std::string, std::string>& headers) {
  this->headers = headers;
}

void HTTPRequest::setBody(const std::string& body) {
  this->body = body;
}

std::string HTTPRequest::getMethod() const {
  return method;
}

std::string HTTPRequest::getUrl() const {
  return url;
}

std::map<std::string, std::string> HTTPRequest::getHeaders() const {
  return headers;
}

std::string HTTPRequest::getBody() const {
  return body;
}

void HTTPRequest::addHeader(const std::string& key, const std::string& value) {
  headers[key] = value;
}

std::string HTTPRequest::getHeader(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator it = headers.find(key);
  if (it != headers.end()) {
    return it->second;
  }
  return "";
}
