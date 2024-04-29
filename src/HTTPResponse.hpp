#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>

class HTTPResponse {
 public:
  HTTPResponse();
  ~HTTPResponse();

  // Getters
  void setStatusCode(int code);
  void setHeaders(const std::map<std::string, std::string>& headers);
  void setBody(const std::string& body);

  // Setters
  int                                getStatusCode() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string                        getBody() const;

 private:
  int                                statusCode;
  std::map<std::string, std::string> headers;
  std::string                        body;
};

#endif
