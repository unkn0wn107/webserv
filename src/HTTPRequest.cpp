/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:58 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 17:13:46 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest() : _method(""), _url(""), _body("") {}

void HTTPRequest::setMethod(const std::string& method) {
  _method = method;
}
void HTTPRequest::setUrl(const std::string& url) {
  _url = url;
}

void HTTPRequest::setHeaders(
    const std::map<std::string, std::string>& headers) {
  _headers = headers;
}

void HTTPRequest::addHeader(const std::string& key, const std::string& value) {
  _headers[key] = value;
}

void HTTPRequest::setBody(const std::string& body) {
  _body = body;
}

std::string HTTPRequest::getRequestStr() const {
  return _requestStr;
}

std::string HTTPRequest::getMethod() const {
  return _method;
}

std::string HTTPRequest::getUrl() const {
  return _url;
}

std::map<std::string, std::string> HTTPRequest::getHeaders() const {
  return _headers;
}

std::string HTTPRequest::getBody() const {
  return _body;
}

std::string HTTPRequest::getHeader(const std::string& key) const {
  std::map<std::string, std::string>::const_iterator it = _headers.find(key);
  if (it != _headers.end()) {
    return it->second;
  }
  return "";
}
