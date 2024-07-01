/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:02 by agaley            #+#    #+#             */
/*   Updated: 2024/06/28 11:03:17 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "Config.hpp"
#include "URI.hpp"

class HTTPRequest {
 public:
  HTTPRequest(std::string rawRequest /*, size_t readn*/);
  ~HTTPRequest();

  static const std::string supportedMethods[4];

  void parseRequest();

  // GETTERS
  std::string                        getSessionId() const;
  std::string                        getRawRequest() const;
  LocationConfig&                    getConfig() const;
  std::string                        getMethod() const;
  std::string                        getURI() const;
  URI::Components                    getURIComponents() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string                        getHeader(const std::string& key) const;
  std::string                        getBody() const;
  std::string                        getProtocol() const;
  std::string                        getHost() const;
  int                                getContentLength() const;

  // SETTERS
  void setSessionId(const std::string& sessionId);
  void setConfig(LocationConfig& config);
  void setMethod(const std::string& method);
  void setURI(const std::string& uri);
  void setHeaders(const std::map<std::string, std::string>& headers);
  void addHeader(const std::string& key, const std::string& value);
  void setBody(const std::string& body);

 private:
  std::string                        _sessionId;
  std::string                        _rawRequest;
  std::string                        _method;
  std::string                        _uri;
  URI::Components                    _uriComponents;
  std::map<std::string, std::string> _headers;
  std::string                        _body;
  std::string                        _protocol;

  void _parseHeaders(std::istringstream& requestStream);
  void _parseSession();
};

#endif
