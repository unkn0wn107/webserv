#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <sstream>
#include <string>

class HTTPRequest {
 public:
  HTTPRequest();
  ~HTTPRequest();

  void parse(const std::string& rawRequest);

  std::string                        getMethod() const;
  std::string                        getUrl() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string                        getHeader(const std::string& key) const;
  std::string                        getBody() const;

 private:
  std::string                        _method;
  std::string                        _url;
  std::map<std::string, std::string> _headers;
  std::string                        _body;
};

#endif
