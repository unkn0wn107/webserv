/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:02 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 17:13:16 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <sstream>
#include <string>

class HTTPRequest {
 public:
  HTTPRequest();
  ~HTTPRequest();

  // GETTERS
  std::string                        getRequestStr() const;
  std::string                        getMethod() const;
  std::string                        getUrl() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string                        getHeader(const std::string& key) const;
  std::string                        getBody() const;

  // SETTERS
  void setMethod(const std::string& method);
  void setUrl(const std::string& url);
  void setHeaders(const std::map<std::string, std::string>& headers);
  void addHeader(const std::string& key, const std::string& value);
  void setBody(const std::string& body);

 private:
  std::string                        _requestStr;
  std::string                        _method;
  std::string                        _url;
  std::map<std::string, std::string> _headers;
  std::string                        _body;
};

#endif
