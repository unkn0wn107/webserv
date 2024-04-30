#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>

class HTTPResponse {
 private:
  int                                _statusCode;
  std::map<std::string, std::string> _headers;
  std::string                        _body;
  std::string                        _protocol;

 public:
  HTTPResponse();
  HTTPResponse(const std::string& protocol);
  ~HTTPResponse();

  // Setters
  void setProtocol(const std::string& protocol);
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

const std::map<int, std::string> HTTPResponse::statusCodeMessages = {
    // 100
    {HTTPResponse::CONTINUE, "Continue"},
    {HTTPResponse::SWITCHING_PROTOCOLS, "Switching Protocols"},
    // 200
    {HTTPResponse::OK, "OK"},
    {HTTPResponse::CREATED, "Created"},
    {HTTPResponse::ACCEPTED, "Accepted"},
    {HTTPResponse::NON_AUTHORITATIVE_INFORMATION,
     "Non-Authoritative Information"},
    {HTTPResponse::NO_CONTENT, "No Content"},
    {HTTPResponse::RESET_CONTENT, "Reset Content"},
    {HTTPResponse::PARTIAL_CONTENT, "Partial Content"},
    // 300
    {HTTPResponse::MULTIPLE_CHOICES, "Multiple Choices"},
    {HTTPResponse::MOVED_PERMANENTLY, "Moved Permanently"},
    {HTTPResponse::FOUND, "Found"},
    {HTTPResponse::SEE_OTHER, "See Other"},
    {HTTPResponse::NOT_MODIFIED, "Not Modified"},
    {HTTPResponse::USE_PROXY, "Use Proxy"},
    {HTTPResponse::TEMPORARY_REDIRECT, "Temporary Redirect"},
    // 400
    {HTTPResponse::BAD_REQUEST, "Bad Request"},
    {HTTPResponse::UNAUTHORIZED, "Unauthorized"},
    {HTTPResponse::PAYMENT_REQUIRED, "Payment Required"},
    {HTTPResponse::FORBIDDEN, "Forbidden"},
    {HTTPResponse::NOT_FOUND, "Not Found"},
    {HTTPResponse::METHOD_NOT_ALLOWED, "Method Not Allowed"},
    {HTTPResponse::NOT_ACCEPTABLE, "Not Acceptable"},
    {HTTPResponse::PROXY_AUTHENTICATION_REQUIRED,
     "Proxy Authentication Required"},
    {HTTPResponse::REQUEST_TIMEOUT, "Request Timeout"},
    {HTTPResponse::CONFLICT, "Conflict"},
    {HTTPResponse::GONE, "Gone"},
    {HTTPResponse::LENGTH_REQUIRED, "Length Required"},
    {HTTPResponse::PRECONDITION_FAILED, "Precondition Failed"},
    {HTTPResponse::REQUEST_ENTITY_TOO_LARGE, "Request Entity Too Large"},
    {HTTPResponse::REQUEST_URI_TOO_LONG, "Request URI Too Long"},
    {HTTPResponse::UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type"},
    {HTTPResponse::REQUESTED_RANGE_NOT_SATISFIABLE,
     "Requested Range Not Satisfiable"},
    {HTTPResponse::EXPECTATION_FAILED, "Expectation Failed"},
    // 500
    {HTTPResponse::INTERNAL_SERVER_ERROR, "Internal Server Error"},
    {HTTPResponse::NOT_IMPLEMENTED, "Not Implemented"},
    {HTTPResponse::BAD_GATEWAY, "Bad Gateway"},
    {HTTPResponse::SERVICE_UNAVAILABLE, "Service Unavailable"},
    {HTTPResponse::GATEWAY_TIMEOUT, "Gateway Timeout"},
    {HTTPResponse::HTTP_VERSION_NOT_SUPPORTED, "HTTP Version Not Supported"}};

#endif
