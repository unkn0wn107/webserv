/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 14:44:30 by mchenava          #+#    #+#             */
/*   Updated: 2024/05/03 12:53:16 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.class.hpp"
#include <sstream>

Response::Response() : _log(Logger::getInstance()), _statusCode(200) {}

Response::Response(const Response& other) :_log(Logger::getInstance()) {
    *this = other;
}

Response& Response::operator=(const Response& other) {
    if (this != &other) {
        _statusCode = other._statusCode;
        _headers = other._headers;
        _body = other._body;
    }
    return *this;
}

Response::~Response() {}

void Response::setStatusCode(int code) {
    _statusCode = code;
}

void Response::setHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void Response::setBody(const std::string& body) {
    _body = body;
}

std::string Response::generate() const {
    std::stringstream responseStream;
    responseStream << "HTTP/1.1 " << _statusCode << " " << "\n";
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        responseStream << it->first << ": " << it->second << "\n";
    }
    responseStream << "\n" << _body;
    return responseStream.str();
}

