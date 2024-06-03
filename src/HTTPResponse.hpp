/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:10 by agaley            #+#    #+#             */
/*   Updated: 2024/06/03 15:14:51 by  mchenava        ###   ########.fr       */
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
  std::string                        _statusMessage;
  std::map<std::string, std::string> _headers;
  std::string                        _body;
  std::string                        _protocol;
  std::map<int, std::string>          _error_pages;
  std::string                        _responseBuffer;

  void _errorResponse();

 public:
  HTTPResponse();
  HTTPResponse(const std::string& protocol);
  HTTPResponse(int statusCode, std::map<int, std::string> error_pages);
  HTTPResponse(int statusCode);
  ~HTTPResponse();

  static std::string getContentType(const std::string& path);

  void  buildResponse();
  int  sendResponse(int clientSocket);
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

  static const std::pair<int, std::string> STATUS_CODE_MESSAGES[];
  static const std::pair<std::string, std::string> CONTENT_TYPES[];
  static const int                         NUM_STATUS_CODE_MESSAGES;

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

#endif
