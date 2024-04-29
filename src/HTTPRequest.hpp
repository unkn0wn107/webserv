#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <string>

class HTTPRequest {
 public:
  HTTPRequest();
  ~HTTPRequest();

  // Getters
  std::string                        getMethod() const;
  std::string                        getUrl() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string                        getBody() const;

  // Setters
  void setMethod(const std::string& method);
  void setUrl(const std::string& url);
  void setHeaders(const std::map<std::string, std::string>& headers);
  void setBody(const std::string& body);

 private:
  std::string                        method;
  std::string                        url;
  std::map<std::string, std::string> headers;
  std::string                        body;
};

#endif
