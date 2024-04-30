/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:10 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:33:09 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <sstream>
#include <string>
#include "Utils.hpp"

class HTTPResponse {
 private:
  int                                _statusCode;
  std::map<std::string, std::string> _headers;
  std::string                        _body;
  std::string                        _protocol;

  static const std::pair<int, std::string> _STATUS_CODE_MESSAGES_PAIRS[];
  static const int                         _NUM_STATUS_CODE_MESSAGES;

 public:
  HTTPResponse();
  HTTPResponse(const std::string& protocol);
  ~HTTPResponse();

  // Setters
  void setStatusCode(int code);
  void setHeaders(const std::map<std::string, std::string>& headers);
  void addHeader(const std::string& key, const std::string& value);
  void setBody(const std::string& body);

  // Getters
  int                                getStatusCode() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string                        getBody() const;
  std::string                        getStatusMessage(int code) const;
  std::string                        generate() const;

  static const std::map<int, std::string> statusCodeMessages;

  static const int CONTINUE = 100;
  static const int SWITCHING_PROTOCOLS = 101;

  static const int OK = 200;
  static const int CREATED = 201;
  static const int ACCEPTED = 202;
  static const int NON_AUTHORITATIVE_INFORMATION = 203;
  static const int NO_CONTENT = 204;
  static const int RESET_CONTENT = 205;
  static const int PARTIAL_CONTENT = 206;

  static const int MULTIPLE_CHOICES = 300;
  static const int MOVED_PERMANENTLY = 301;
  static const int FOUND = 302;
  static const int SEE_OTHER = 303;
  static const int NOT_MODIFIED = 304;
  static const int USE_PROXY = 305;
  static const int TEMPORARY_REDIRECT = 307;

  static const int BAD_REQUEST = 400;
  static const int UNAUTHORIZED = 401;
  static const int PAYMENT_REQUIRED = 402;
  static const int FORBIDDEN = 403;
  static const int NOT_FOUND = 404;
  static const int METHOD_NOT_ALLOWED = 405;
  static const int NOT_ACCEPTABLE = 406;
  static const int PROXY_AUTHENTICATION_REQUIRED = 407;
  static const int REQUEST_TIMEOUT = 408;
  static const int CONFLICT = 409;
  static const int GONE = 410;
  static const int LENGTH_REQUIRED = 411;
  static const int PRECONDITION_FAILED = 412;
  static const int REQUEST_ENTITY_TOO_LARGE = 413;
  static const int REQUEST_URI_TOO_LONG = 414;
  static const int UNSUPPORTED_MEDIA_TYPE = 415;
  static const int REQUESTED_RANGE_NOT_SATISFIABLE = 416;
  static const int EXPECTATION_FAILED = 417;

  static const int INTERNAL_SERVER_ERROR = 500;
  static const int NOT_IMPLEMENTED = 501;
  static const int BAD_GATEWAY = 502;
  static const int SERVICE_UNAVAILABLE = 503;
  static const int GATEWAY_TIMEOUT = 504;
  static const int HTTP_VERSION_NOT_SUPPORTED = 505;
};

// Define the array of pairs and its size
const std::pair<int, std::string> HTTPResponse::_STATUS_CODE_MESSAGES_PAIRS[] =
    {std::make_pair(HTTPResponse::CONTINUE, "Continue"),
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

const int HTTPResponse::_NUM_STATUS_CODE_MESSAGES =
    sizeof(_STATUS_CODE_MESSAGES_PAIRS) / sizeof(std::pair<int, std::string>);

const std::map<int, std::string> HTTPResponse::statusCodeMessages(
    _STATUS_CODE_MESSAGES_PAIRS,
    _STATUS_CODE_MESSAGES_PAIRS + HTTPResponse::_NUM_STATUS_CODE_MESSAGES);

#endif
