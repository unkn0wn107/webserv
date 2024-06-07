/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exception.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 03:52:01 by agaley            #+#    #+#             */
/*   Updated: 2024/06/07 03:52:03 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Exception.hpp"
#include "Logger.hpp"

Exception::Exception(const std::string& message) : _message(message) {
  Logger::getInstance().error("Exception: " + message);
}

Exception::~Exception() throw() {}

const char* Exception::what() const throw() {
  return _message.c_str();
}
