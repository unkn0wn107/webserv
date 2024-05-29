/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:55 by agaley            #+#    #+#             */
/*   Updated: 2024/05/25 18:31:47 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLER_H
#define HANDLER_H

#include "Config.hpp"
#include "Logger.hpp"

class HTTPRequest;
class HTTPResponse;

class Handler {
 protected:
  Logger& _log;

 public:
  Handler() : _log(Logger::getInstance()) {}
  virtual ~Handler() {}
  virtual HTTPResponse handle(const HTTPRequest& request) = 0;
};

#endif
