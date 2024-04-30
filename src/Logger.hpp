/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:12:22 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:51:39 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

class Logger {
 public:
  static Logger& get();

  const enum logLevel { INFO, WARNING, ERROR };

  /**
   * Logs a message with a severity level.
   * @param message The message to log.
   * @param level The severity level of the message.
   */
  void log(const std::string& message, logLevel level);

 private:
  Logger();
  ~Logger();
  Logger(const Logger&);
  Logger& operator=(const Logger&);
};

#endif
