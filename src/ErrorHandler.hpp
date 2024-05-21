/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:33 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:11:34 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

class ErrorHandler {
 public:
  ErrorHandler();

  /**
   * Handles exceptions and errors by logging them and optionally
   * generating an appropriate response.
   * @param e The exception object caught.
   */
  static void exception(const std::exception& e);

  /**
   * Handles generic error messages.
   * @param errorMessage The error message to be logged.
   */
  static void log(const std::string& errorMessage);

  /**
   * Handles fatal errors by logging them and optionally
   * generating an appropriate response.
   * @param errorMessage The error message to be logged.
   */
  static void fatal(const std::string& errorMessage);
};

#endif
