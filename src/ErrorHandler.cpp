/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:29 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:11:30 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorHandler.hpp"

ErrorHandler::ErrorHandler() {}

void ErrorHandler::exception(const std::exception& ex) {
  std::cerr << "Exception: " << ex.what() << std::endl;
}

void ErrorHandler::log(const std::string& errorMessage) {
  std::cerr << "Error: " << errorMessage << std::endl;
}

void ErrorHandler::fatal(const std::string& errorMessage) {
  std::cerr << "Fatal Error: " << errorMessage << std::endl;
  std::exit(EXIT_FAILURE);
}
