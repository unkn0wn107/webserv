/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:13:45 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 17:05:39 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerException.class.hpp"

ServerException::ServerException() : _log(Logger::getInstance()) {}

ServerException::ServerException(const std::string& msg) : _log(Logger::getInstance()), _message(msg) {}

ServerException::ServerException(const ServerException& other) : _log(Logger::getInstance()) {
    *this = other;
}

ServerException& ServerException::operator=(const ServerException& other) {
    _message = other._message;
    return *this;
}

ServerException::~ServerException() throw() {}

const char* ServerException::what() const throw() {
    _log.error(_message);
    return _message.c_str();
}
