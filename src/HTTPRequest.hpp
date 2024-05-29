/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:02 by agaley            #+#    #+#             */
/*   Updated: 2024/05/29 16:08:27 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <sstream>
#include <string>
#include "Config.hpp"

class HTTPRequest {
 public:
  HTTPRequest(std::string rawRequest, size_t readn);
  ~HTTPRequest();

  void	parseRequest();

  // GETTERS
  std::string													getRawRequest() const;
  std::string                        	getMethod() const;
  std::string                        	getURI() const;
  std::map<std::string, std::string>	getHeaders() const;
  std::string                        	getHeader(const std::string& key) const;
  std::string                        	getBody() const;
  std::string                        	getProtocol() const;
  std::string													getHost() const;
  void																configure(ServerConfig& config);

  // SETTERS
  void setMethod(const std::string& method);
  void setURI(const std::string& uri);
  void setHeaders(const std::map<std::string, std::string>& headers);
  void addHeader(const std::string& key, const std::string& value);
  void setBody(const std::string& body);

 private:
  std::string                        	_rawRequest;
  size_t                              _readn;
  std::string                        	_method;
  std::string                        	_uri;
  std::map<std::string, std::string>	_headers;
  std::string                         _body;
  std::string                         _protocol;
	ServerConfig												_config;
};

#endif
