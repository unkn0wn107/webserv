#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>

/**
 * Class HTTPResponse represents an HTTP response.
 * It encapsulates the status code, headers, and body of the response.
 */
class HTTPResponse {
 public:
  HTTPResponse();
  ~HTTPResponse();

  /**
   * Set the status code of the HTTP response.
   * @param code The status code as an integer.
   */
  void setStatusCode(int code);

  /**
   * Get the status code of the HTTP response.
   * @return The status code as an integer.
   */
  int getStatusCode() const;

  /**
   * Set the headers of the HTTP response.
   * @param headers A map of header fields and values.
   */
  void setHeaders(const std::map<std::string, std::string>& headers);

  /**
   * Get the headers of the HTTP response.
   * @return A map of header fields and values.
   */
  std::map<std::string, std::string> getHeaders() const;

  /**
   * Set the body of the HTTP response.
   * @param body The body content as a string.
   */
  void setBody(const std::string& body);

  /**
   * Get the body of the HTTP response.
   * @return The body content as a string.
   */
  std::string getBody() const;

 private:
  int                                statusCode;  // HTTP status code
  std::map<std::string, std::string> headers;     // HTTP headers
  std::string                        body;        // HTTP body content
};

#endif  // HTTPRESPONSE_H
