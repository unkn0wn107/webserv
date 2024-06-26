/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:07 by agaley            #+#    #+#             */
/*   Updated: 2024/06/27 15:00:37 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPResponse.hpp"
#include <sys/sendfile.h>
#include <sys/socket.h>
#include "Utils.hpp"
#include "VirtualServer.hpp"

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

const std::pair<std::string, std::string> HTTPResponse::CONTENT_TYPES[] = {
    std::make_pair("aac", "audio/aac"),
    std::make_pair("abw", "application/x-abiword"),
    std::make_pair("apng", "image/apng"),
    std::make_pair("arc", "application/x-freearc"),
    std::make_pair("avif", "image/avif"),
    std::make_pair("avi", "video/x-msvideo"),
    std::make_pair("azw", "application/vnd.amazon.ebook"),
    std::make_pair("bin", "application/octet-stream"),
    std::make_pair("bmp", "image/bmp"),
    std::make_pair("bz", "application/x-bzip"),
    std::make_pair("bz2", "application/x-bzip2"),
    std::make_pair("cda", "application/x-cdf"),
    std::make_pair("csh", "application/x-csh"),
    std::make_pair("css", "text/css"),
    std::make_pair("csv", "text/csv"),
    std::make_pair("doc", "application/msword"),
    std::make_pair(
        "docx",
        "application/"
        "vnd.openxmlformats-officedocument.wordprocessingml.document"),
    std::make_pair("eot", "application/vnd.ms-fontobject"),
    std::make_pair("epub", "application/epub+zip"),
    std::make_pair("gz", "application/gzip"),
    std::make_pair("gif", "image/gif"),
    std::make_pair("htm", "text/html"),
    std::make_pair("html", "text/html"),
    std::make_pair("ico", "image/vnd.microsoft.icon"),
    std::make_pair("ics", "text/calendar"),
    std::make_pair("jar", "application/java-archive"),
    std::make_pair("jpeg", "image/jpeg"),
    std::make_pair("jpg", "image/jpeg"),
    std::make_pair("js", "text/javascript"),
    std::make_pair("json", "application/json"),
    std::make_pair("jsonld", "application/ld+json"),
    std::make_pair("mid", "audio/midi"),
    std::make_pair("midi", "audio/x-midi"),
    std::make_pair("mjs", "text/javascript"),
    std::make_pair("mp3", "audio/mpeg"),
    std::make_pair("mp4", "video/mp4"),
    std::make_pair("mpeg", "video/mpeg"),
    std::make_pair("mpkg", "application/vnd.apple.installer+xml"),
    std::make_pair("odp", "application/vnd.oasis.opendocument.presentation"),
    std::make_pair("ods", "application/vnd.oasis.opendocument.spreadsheet"),
    std::make_pair("odt", "application/vnd.oasis.opendocument.text"),
    std::make_pair("oga", "audio/ogg"),
    std::make_pair("ogv", "video/ogg"),
    std::make_pair("ogx", "application/ogg"),
    std::make_pair("opus", "audio/opus"),
    std::make_pair("otf", "font/otf"),
    std::make_pair("png", "image/png"),
    std::make_pair("pdf", "application/pdf"),
    std::make_pair("php", "application/x-httpd-php"),
    std::make_pair("ppt", "application/vnd.ms-powerpoint"),
    std::make_pair(
        "pptx",
        "application/"
        "vnd.openxmlformats-officedocument.presentationml.presentation"),
    std::make_pair("rar", "application/vnd.rar"),
    std::make_pair("rtf", "application/rtf"),
    std::make_pair("sh", "application/x-sh"),
    std::make_pair("svg", "image/svg+xml"),
    std::make_pair("tar", "application/x-tar"),
    std::make_pair("tif", "image/tiff"),
    std::make_pair("tiff", "image/tiff"),
    std::make_pair("ts", "video/mp2t"),
    std::make_pair("ttf", "font/ttf"),
    std::make_pair("txt", "text/plain"),
    std::make_pair("vsd", "application/vnd.visio"),
    std::make_pair("wav", "audio/wav"),
    std::make_pair("weba", "audio/webm"),
    std::make_pair("webm", "video/webm"),
    std::make_pair("webp", "image/webp"),
    std::make_pair("woff", "font/woff"),
    std::make_pair("woff2", "font/woff2"),
    std::make_pair("xhtml", "application/xhtml+xml"),
    std::make_pair("xls", "application/vnd.ms-excel"),
    std::make_pair(
        "xlsx",
        "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
    std::make_pair("xml", "application/xml"),
    std::make_pair("xul", "application/vnd.mozilla.xul+xml"),
    std::make_pair("zip", "application/zip"),
    std::make_pair("3gp", "video/3gpp"),
    std::make_pair("3g2", "video/3gpp2"),
    std::make_pair("7z", "application/x-7z-compressed")};

std::pair<int, std::string> HTTPResponse::_defaultErrorPages[] = {
    std::make_pair(301, std::string(ERR_PAGE_301)),
    std::make_pair(302, std::string(ERR_PAGE_302)),
    std::make_pair(307, std::string(ERR_PAGE_307)),
    std::make_pair(400, std::string(ERR_PAGE_400)),
    std::make_pair(403, std::string(ERR_PAGE_403)),
    std::make_pair(404, std::string(ERR_PAGE_404)),
    std::make_pair(405, std::string(ERR_PAGE_405)),
    std::make_pair(500, std::string(ERR_PAGE_500)),
    std::make_pair(501, std::string(ERR_PAGE_501)),
    std::make_pair(502, std::string(ERR_PAGE_502)),
    std::make_pair(503, std::string(ERR_PAGE_503)),
    std::make_pair(504, std::string(ERR_PAGE_504)),
    std::make_pair(505, std::string(ERR_PAGE_505))};


HTTPResponse::HTTPResponse()
    : _log(Logger::getInstance()),
      _statusCode(HTTPResponse::OK),
      _config(NULL) {
  _protocol = "HTTP/1.1";
}

HTTPResponse::~HTTPResponse() {
  _headers.clear();
  _body.clear();
  _file.clear();
  _responseBuffer.clear();
}

HTTPResponse& HTTPResponse::operator=(const HTTPResponse& other) {
  if (this != &other) {
    _log = other._log;
    _statusCode = other._statusCode;
    _statusMessage = other._statusMessage;
    _headers = other._headers;
    _body = other._body;
    _file = other._file;
    _protocol = other._protocol;
    _config = other._config;
  }
  return *this;
}

HTTPResponse::HTTPResponse(const std::string& protocol)
    : _log(Logger::getInstance()),
      _statusCode(HTTPResponse::OK),
      _protocol(protocol),
      _config(NULL),
      _responseBufferSize(0),
      _responseBufferPos(0),
      _responseFilePos(0),
      _fileSize(0)
{}

HTTPResponse::HTTPResponse(int statusCode, LocationConfig* config)
    : _log(Logger::getInstance()),
      _statusCode(statusCode),
      _statusMessage(getStatusMessage(statusCode)),
      _headers(std::map<std::string, std::string>()),
      _body(""),
      _file(""),
      _protocol("HTTP/1.1"),
      _config(config),
      _responseBufferSize(0),
      _responseBufferPos(0),
      _responseFilePos(0),
      _fileSize(0) {
  if (statusCode < 100 || statusCode > 599) {
    throw std::invalid_argument("Invalid status code");
  }
  if (statusCode >= 300 && statusCode < 400)
    addHeader("Location", _config->returnUrl);
}

HTTPResponse::HTTPResponse(int statusCode)
    : _log(Logger::getInstance()),
      _statusCode(statusCode),
      _statusMessage(getStatusMessage(statusCode)),
      _headers(std::map<std::string, std::string>()),
      _body(""),
      _file(""),
      _protocol("HTTP/1.1"),
      _config(NULL),
      _responseBufferSize(0),
      _responseBufferPos(0),
      _responseFilePos(0),
      _fileSize(0) {
  if (statusCode != 200 && statusCode != 201) {
    throw std::invalid_argument("Invalid status code : " +
                                Utils::to_string(statusCode));
  }
}

void HTTPResponse::_errorResponse() {
  if (_config) {
    std::map<int, std::string>::const_iterator it = _config->error_pages.find(_statusCode);
    if (_statusCode >= 300) {
    if (it != _config->error_pages.end()) {
      _file = _config->root + it->second;
      _log.info("Error page: " + _file);
    } else {
      _log.info("Default error page");
      _body = HTTPResponse::defaultErrorPage(_statusCode);
    }
  }
  addHeader("Content-Type", "text/html");
  if (!_file.empty() || !_body.empty()) {
    addHeader("Content-Length",
              _file.empty()
                  ? Utils::to_string(_body.length())
                  : Utils::to_string(FileManager::getFileSize(_file)));
    }
  }
}

void HTTPResponse::buildResponse() {
  _log.info("HTTPResponse: buildResponse status: " + Utils::to_string(_statusCode));
  _errorResponse();
  if (_responseBuffer.empty()) {
    _responseBuffer = "HTTP/1.1 " + Utils::to_string(_statusCode) + " " +
                      _statusMessage + "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
      _responseBuffer += it->first + ": " + it->second + "\r\n";
    }
    _responseBuffer += "\r\n" + _body;
    _responseBufferSize = _responseBuffer.size();
  }
  if (_file.empty())
    return;
  _fileSize = FileManager::getFileSize(_file);
}

std::string HTTPResponse::defaultErrorPage(int status) {
  for (unsigned long i = 0;
       i < sizeof(_defaultErrorPages) / sizeof(_defaultErrorPages[0]); i++) {
    if (_defaultErrorPages[i].first == status) {
      return std::string(_defaultErrorPages[i].second);
    }
  }
  return std::string("");
}

ssize_t HTTPResponse::_send(int socket, size_t sndbuf) {
  ssize_t bytesSent;
  size_t remaining = _responseBuffer.size() - _responseBufferPos;

  if (sndbuf > remaining) {
    sndbuf = remaining;
  }

  bytesSent = send(socket, _responseBuffer.c_str() + _responseBufferPos, sndbuf, 0);
  if (bytesSent == -1) {
    usleep(1000);
    return -1;
  }
  _log.info("HTTPResponse: _send bytesSent: " + Utils::to_string(bytesSent) +
            " remaining: " + Utils::to_string(remaining) +
            " _responseBufferPos: " + Utils::to_string(_responseBufferPos) +
            " _responseBufferSize: " + Utils::to_string(_responseBufferSize));
  _responseBufferPos += bytesSent;
  return _responseBufferPos;
}

void HTTPResponse::_sendfile(int clientSocket, FILE* file, size_t sndbuf) {
  ssize_t bytesSent;
  bytesSent = sendfile(clientSocket, fileno(file), &_responseFilePos, sndbuf);
  if (bytesSent == -1)
    usleep(1000);
  _log.info("HTTPResponse: _sendfile bytesSent: " + Utils::to_string(bytesSent) +
            " _responseFilePos: " + Utils::to_string(_responseFilePos));
}

int HTTPResponse::sendResponse(int clientSocket, size_t sndbuf) {
  buildResponse();
  if (_send(clientSocket, sndbuf) == -1)
    return -1;
  if (_responseBufferPos == _responseBufferSize && _file.empty())
    return 1;
  _toSend = fopen(_file.c_str(), "r");
  if (!_toSend){
    sendResponse(500, clientSocket);
    throw Exception("(fopen) Error opening file" + std::string(strerror(errno)));
  }
  _sendfile(clientSocket, _toSend, sndbuf);
  fclose(_toSend);
  if (static_cast<size_t>(_responseFilePos) == _fileSize)
    return 1;
  return 0;
}

int HTTPResponse::sendResponse(int statusCode, int clientSocket) {
  std::string statusLine = "HTTP/1.1 " + Utils::to_string(statusCode) + " " +
                           getStatusMessage(statusCode) + "\r\n";
  std::string headers = "Content-Type: text/html\r\n";
  std::string body = defaultErrorPage(statusCode);
  headers += "Content-Length: " + Utils::to_string(body.length()) + "\r\n\r\n";
  std::string response = statusLine + headers + body;
  if (send(clientSocket, response.c_str(), response.length(), 0) == -1) {
    return -1;
  }
  return 0;
}

std::string HTTPResponse::getExtensionFromContentType(
    const std::string& contentType) {
  for (size_t i = 0; i < sizeof(CONTENT_TYPES) / sizeof(CONTENT_TYPES[0]);
       ++i) {
    if (CONTENT_TYPES[i].second == contentType) {
      return CONTENT_TYPES[i].first;
    }
  }
  return "";
}

std::string HTTPResponse::getContentType(const std::string& path) {
  std::string  extension = path.substr(path.find_last_of('.') + 1);
  const size_t numContentTypes =
      sizeof(CONTENT_TYPES) / sizeof(CONTENT_TYPES[0]);

  for (size_t i = 0; i < numContentTypes; ++i) {
    if (CONTENT_TYPES[i].first == extension) {
      return CONTENT_TYPES[i].second;
    }
  }
  return "application/octet-stream";
}

void HTTPResponse::setCookie(const std::string& key, const std::string& value) {
  addHeader("Set-Cookie", key + "=" + value + "; Path=/; HttpOnly");
}

void HTTPResponse::setStatusCode(int code) {
  _statusCode = code;
}

void HTTPResponse::setFile(const std::string& path) {
  _file = path;
}

void HTTPResponse::setHeaders(const std::map<std::string, std::string>& hdrs) {
  _headers = hdrs;
}

void HTTPResponse::addHeader(const std::string& key, const std::string& value) {
  _headers[key] = value;
}

void HTTPResponse::deleteHeader(const std::string& key) {
  _headers.erase(key);
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

std::string HTTPResponse::getStatusMessage(int code) {
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

  if (_headers.find("Content-Type") == _headers.end())
    response += "Content-Type: text/plain\r\n";

  if (_body.length() > 0) {
    response += "Content-Length: " + Utils::to_string(_body.length()) + "\r\n";
    response += "\r\n" + _body;
  }

  return response;
}
