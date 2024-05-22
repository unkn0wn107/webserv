/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:07 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 19:58:27 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPResponse.hpp"

const int HTTPResponse::CONTINUE;
const int HTTPResponse::SWITCHING_PROTOCOLS;

const int HTTPResponse::OK;
const int HTTPResponse::CREATED;
const int HTTPResponse::ACCEPTED;
const int HTTPResponse::NON_AUTHORITATIVE_INFORMATION;
const int HTTPResponse::NO_CONTENT;
const int HTTPResponse::RESET_CONTENT;
const int HTTPResponse::PARTIAL_CONTENT;

const int HTTPResponse::MULTIPLE_CHOICES;
const int HTTPResponse::MOVED_PERMANENTLY;
const int HTTPResponse::FOUND;
const int HTTPResponse::SEE_OTHER;
const int HTTPResponse::NOT_MODIFIED;
const int HTTPResponse::USE_PROXY;
const int HTTPResponse::TEMPORARY_REDIRECT;

const int HTTPResponse::BAD_REQUEST;
const int HTTPResponse::UNAUTHORIZED;
const int HTTPResponse::PAYMENT_REQUIRED;
const int HTTPResponse::FORBIDDEN;
const int HTTPResponse::NOT_FOUND;
const int HTTPResponse::METHOD_NOT_ALLOWED;
const int HTTPResponse::NOT_ACCEPTABLE;
const int HTTPResponse::PROXY_AUTHENTICATION_REQUIRED;
const int HTTPResponse::REQUEST_TIMEOUT;
const int HTTPResponse::CONFLICT;
const int HTTPResponse::GONE;
const int HTTPResponse::LENGTH_REQUIRED;
const int HTTPResponse::PRECONDITION_FAILED;
const int HTTPResponse::REQUEST_ENTITY_TOO_LARGE;
const int HTTPResponse::REQUEST_URI_TOO_LONG;
const int HTTPResponse::UNSUPPORTED_MEDIA_TYPE;
const int HTTPResponse::REQUESTED_RANGE_NOT_SATISFIABLE;
const int HTTPResponse::EXPECTATION_FAILED;

const int HTTPResponse::INTERNAL_SERVER_ERROR;
const int HTTPResponse::NOT_IMPLEMENTED;
const int HTTPResponse::BAD_GATEWAY;
const int HTTPResponse::SERVICE_UNAVAILABLE;
const int HTTPResponse::GATEWAY_TIMEOUT;
const int HTTPResponse::HTTP_VERSION_NOT_SUPPORTED;

const std::pair<int, std::string> HTTPResponse::STATUS_CODE_MESSAGES[] = {
    std::make_pair(HTTPResponse::CONTINUE, "Continue"),
    std::make_pair(HTTPResponse::SWITCHING_PROTOCOLS, "Switching Protocols"),
    // 200
    std::make_pair(HTTPResponse::OK, "OK"),
    std::make_pair(HTTPResponse::CREATED, "Created"),
    std::make_pair(HTTPResponse::ACCEPTED, "Accepted"),
    std::make_pair(HTTPResponse::NON_AUTHORITATIVE_INFORMATION,
                   "Non-Authoritative Information"),
    std::make_pair(HTTPResponse::NO_CONTENT, "No Content"),
    std::make_pair(HTTPResponse::RESET_CONTENT, "Reset Content"),
    std::make_pair(HTTPResponse::PARTIAL_CONTENT, "Partial Content"),
    // 300
    std::make_pair(HTTPResponse::MULTIPLE_CHOICES, "Multiple Choices"),
    std::make_pair(HTTPResponse::MOVED_PERMANENTLY, "Moved Permanently"),
    std::make_pair(HTTPResponse::FOUND, "Found"),
    std::make_pair(HTTPResponse::SEE_OTHER, "See Other"),
    std::make_pair(HTTPResponse::NOT_MODIFIED, "Not Modified"),
    std::make_pair(HTTPResponse::USE_PROXY, "Use Proxy"),
    std::make_pair(HTTPResponse::TEMPORARY_REDIRECT, "Temporary Redirect"),
    // 400
    std::make_pair(HTTPResponse::BAD_REQUEST, "Bad Request"),
    std::make_pair(HTTPResponse::UNAUTHORIZED, "Unauthorized"),
    std::make_pair(HTTPResponse::PAYMENT_REQUIRED, "Payment Required"),
    std::make_pair(HTTPResponse::FORBIDDEN, "Forbidden"),
    std::make_pair(HTTPResponse::NOT_FOUND, "Not Found"),
    std::make_pair(HTTPResponse::METHOD_NOT_ALLOWED, "Method Not Allowed"),
    std::make_pair(HTTPResponse::NOT_ACCEPTABLE, "Not Acceptable"),
    std::make_pair(HTTPResponse::PROXY_AUTHENTICATION_REQUIRED,
                   "Proxy Authentication Required"),
    std::make_pair(HTTPResponse::REQUEST_TIMEOUT, "Request Timeout"),
    std::make_pair(HTTPResponse::CONFLICT, "Conflict"),
    std::make_pair(HTTPResponse::GONE, "Gone"),
    std::make_pair(HTTPResponse::LENGTH_REQUIRED, "Length Required"),
    std::make_pair(HTTPResponse::PRECONDITION_FAILED, "Precondition Failed"),
    std::make_pair(HTTPResponse::REQUEST_ENTITY_TOO_LARGE,
                   "Request Entity Too Large"),
    std::make_pair(HTTPResponse::REQUEST_URI_TOO_LONG, "Request URI Too Long"),
    std::make_pair(HTTPResponse::UNSUPPORTED_MEDIA_TYPE,
                   "Unsupported Media Type"),
    std::make_pair(HTTPResponse::REQUESTED_RANGE_NOT_SATISFIABLE,
                   "Requested Range Not Satisfiable"),
    std::make_pair(HTTPResponse::EXPECTATION_FAILED, "Expectation Failed"),
    // 500
    std::make_pair(HTTPResponse::INTERNAL_SERVER_ERROR,
                   "Internal Server Error"),
    std::make_pair(HTTPResponse::NOT_IMPLEMENTED, "Not Implemented"),
    std::make_pair(HTTPResponse::BAD_GATEWAY, "Bad Gateway"),
    std::make_pair(HTTPResponse::SERVICE_UNAVAILABLE, "Service Unavailable"),
    std::make_pair(HTTPResponse::GATEWAY_TIMEOUT, "Gateway Timeout"),
    std::make_pair(HTTPResponse::HTTP_VERSION_NOT_SUPPORTED,
                   "HTTP Version Not Supported")};

const int HTTPResponse::NUM_STATUS_CODE_MESSAGES =
    sizeof(HTTPResponse::STATUS_CODE_MESSAGES) /
    sizeof(HTTPResponse::STATUS_CODE_MESSAGES[0]);

HTTPResponse::HTTPResponse() : _statusCode(HTTPResponse::OK) {
  _protocol = "HTTP/1.1";
}

HTTPResponse::~HTTPResponse() {}

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
  for (int i = 0; i < HTTPResponse::NUM_STATUS_CODE_MESSAGES; ++i) {
    if (HTTPResponse::STATUS_CODE_MESSAGES[i].first == code) {
      return HTTPResponse::STATUS_CODE_MESSAGES[i].second;
    }
  }
  return "";
}

std::string HTTPResponse::generate() const {
  std::string response;
  response += "HTTP/1.1 " + Utils::to_string(_statusCode) + " " +
              getStatusMessage(_statusCode) + "\r\n";

  for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
       it != _headers.end(); ++it) {
    response += it->first + ": " + it->second + "\r\n";
  }

  response += "Content-Type: text/plain\r\n";

  if (_body.length() > 0) {
    response += "Content-Length: " + Utils::to_string(_body.length()) + "\r\n";
    response += "\r\n" + _body;
  }

  return response;
}
