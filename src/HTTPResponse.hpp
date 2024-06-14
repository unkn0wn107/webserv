/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:10 by agaley            #+#    #+#             */
/*   Updated: 2024/06/14 13:39:20 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <sstream>
#include <string>
#include "Logger.hpp"
#include "Utils.hpp"

#define ERR_PAGE_301 \
  "<!DOCTYPE html><html><body><h1>301 Moved Permanently</h1></body></html>"
#define ERR_PAGE_302 \
  "<!DOCTYPE html><html><body><h1>302 Found</h1></body></html>"
#define ERR_PAGE_307 \
  "<!DOCTYPE html><html><body><h1>307 Temporary Redirect</h1></body></html>"
#define ERR_PAGE_400                                                           \
  "<!DOCTYPE html><html><body><h1>400 Bad Request</h1><p>Your browser sent a " \
  "request that this server could not understand.</p></body></html>"
#define ERR_PAGE_403                                                    \
  "<!DOCTYPE html><html><body><h1>403 Forbidden</h1><p>You don't have " \
  "permission to access this resource.</p></body></html>"
#define ERR_PAGE_404                                                   \
  "<!DOCTYPE html><html><body><h1>404 Not Found</h1><p>The requested " \
  "resource could not be found on this server.</p></body></html>"
#define ERR_PAGE_405                                                          \
  "<!DOCTYPE html><html><body><h1>405 Method Not Allowed</h1><p>The request " \
  "method is not supported for the requested resource.</p></body></html>"
#define ERR_PAGE_500                                                      \
  "<!DOCTYPE html><html><body><h1>500 Internal Server Error</h1><p>The "  \
  "server encountered an internal error and was unable to complete your " \
  "request.</p></body></html>"
#define ERR_PAGE_501                                                           \
  "<!DOCTYPE html><html><body><h1>501 Not Implemented</h1><p>The server does " \
  "not support the functionality required to fulfill the "                     \
  "request.</p></body></html>"
#define ERR_PAGE_502                                                           \
  "<!DOCTYPE html><html><body><h1>502 Bad Gateway</h1><p>The server received " \
  "an invalid response from the upstream server.</p></body></html>"
#define ERR_PAGE_503                                                           \
  "<!DOCTYPE html><html><body><h1>503 Service Unavailable</h1><p>The server "  \
  "is currently unable to handle the request due to temporary overloading or " \
  "maintenance of the server.</p></body></html>"
#define ERR_PAGE_504                                                          \
  "<!DOCTYPE html><html><body><h1>504 Gateway Timeout</h1><p>The server did " \
  "not receive a timely response from the upstream server.</p></body></html>"
#define ERR_PAGE_505                                                          \
  "<!DOCTYPE html><html><body><h1>505 HTTP Version Not Supported</h1><p>The " \
  "server does not support the HTTP protocol version used in the "            \
  "request.</p></body></html>"

class HTTPResponse {
 private:
  Logger&                            _log;
  int                                _statusCode;
  std::string                        _statusMessage;
  std::map<std::string, std::string> _headers;
  std::string                        _body;
  std::string                        _file;
  std::string                        _protocol;
  std::map<int, std::string>         _error_pages;
  std::string                        _responseBuffer;
  static std::pair<int, std::string> _defaultErrorPages[];

  void _errorResponse();

 public:
  HTTPResponse();
  HTTPResponse(int statusCode);
  HTTPResponse(const std::string& protocol);
  HTTPResponse(int statusCode, std::map<int, std::string> error_pages);
  HTTPResponse(int statusCode, LocationConfig& config, std::string redirectUrl);
  HTTPResponse(int                                statusCode,
               std::map<std::string, std::string> headers,
               std::string                        body);
  ~HTTPResponse();

  static std::string getContentType(const std::string& path);
  static std::string getExtensionFromContentType(
      const std::string& contentType);

  void               buildResponse();
  int                sendResponse(int clientSocket);
  static int         sendResponse(int statusCode, int clientSocket);
  static std::string defaultErrorPage(int status);
  // Setters
  void setCookie(const std::string& key, const std::string& value);
  void setStatusCode(int code);
  void setHeaders(const std::map<std::string, std::string>& headers);
  void addHeader(const std::string& key, const std::string& value);
  void setBody(const std::string& body);
  void setFile(const std::string& path);

  // Getters
  int                                getStatusCode() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string                        getBody() const;
  static std::string                 getStatusMessage(int code);
  std::string                        generate() const;

  static const std::pair<int, std::string>         STATUS_CODE_MESSAGES[];
  static const std::pair<std::string, std::string> CONTENT_TYPES[];
  static const int                                 NUM_STATUS_CODE_MESSAGES;

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
