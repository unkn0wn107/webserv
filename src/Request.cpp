/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 14:41:58 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 17:07:50 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.class.hpp"

Request::Request() : _log(Logger::getInstance()), _method(""), _uri(""), _body("") {}

Request::Request(const std::string& rawRequest) : _log(Logger::getInstance()) {
    parse(rawRequest, rawRequest.size());
}

Request::Request(const Request& other) : _log(Logger::getInstance()) {
	*this = other;
}

Request& Request::operator=(const Request& other) {
    if (this != &other) {
        _method = other._method;
        _uri = other._uri;
        _headers = other._headers;
        _body = other._body;
    }
    return *this;
}

Request::~Request() {}

std::string Request::getMethod() const {
    return _method;
}

std::string Request::getURI() const {
    return _uri;
}

std::string Request::getHeader(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

std::string Request::getBody() const {
    return _body;
}

bool Request::isEmpty() const {
    return _method.empty() && _uri.empty() && _headers.empty() && _body.empty();
}

void Request::parse(std::string buffer, int bytesReceived) {
    // Simplified parsing logic
    // In a real scenario, you would parse the HTTP request line by line
    _method = "GET"; // Example method
    _uri = "/"; // Example URI
    // Example header
    _headers["Content-Type"] = "text/html";
    _body = buffer.substr(0, bytesReceived); // Simplified example
}
