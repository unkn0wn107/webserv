#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse()
    : statusCode(200) {  // Default status code is 200 OK
}

void HTTPResponse::setStatusCode(int code) {
  statusCode = code;
}

int HTTPResponse::getStatusCode() const {
  return statusCode;
}

void HTTPResponse::setHeaders(const std::map<std::string, std::string>& hdrs) {
  headers = hdrs;
}

std::map<std::string, std::string> HTTPResponse::getHeaders() const {
  return headers;
}

void HTTPResponse::setBody(const std::string& bdy) {
  body = bdy;
}

std::string HTTPResponse::getBody() const {
  return body;
}

std::string HTTPResponse::toString() const {
  std::string response;
  response += "HTTP/1.1 " + std::to_string(statusCode) + " " +
              getStatusMessage(statusCode) + "\r\n";

  for (const auto& header : headers) {
    response += header.first + ": " + header.second + "\r\n";
  }

  response += "\r\n" + body;

  return response;
}

std::string HTTPResponse::getStatusMessage(int code) const {
  switch (code) {
    case 200:
      return "OK";
    case 400:
      return "Bad Request";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    case 500:
      return "Internal Server Error";
    default:
      return "Unknown";
  }
}
