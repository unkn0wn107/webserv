/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exception.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/07 03:52:01 by agaley            #+#    #+#             */
/*   Updated: 2024/07/03 17:36:32 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Exception.hpp"
#include <iostream>

Exception::Exception(const std::string& message) : _message(message) {
  std::cerr << "Exception: " << message << std::endl;
}

Exception::~Exception() throw() {}

const char* Exception::what() const throw() {
  return _message.c_str();
}
