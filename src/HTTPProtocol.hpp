/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPProtocol.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:40 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 17:00:53 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPROTOCOL_H
#define HTTPPROTOCOL_H

#include <string>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

const enum Version { UNKNOWN, HTTP1_1 };

class HTTPProtocol {
 public:
  HTTPProtocol() {}
  virtual ~HTTPProtocol() {}

  Version version;

  virtual HTTPRequest  parseRequest(const std::string& requestString) = 0;
  virtual HTTPResponse createResponse() = 0;
  virtual HTTPResponse processRequest(const HTTPRequest& request) = 0;
};

#endif
