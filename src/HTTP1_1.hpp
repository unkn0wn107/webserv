/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP1_1.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:10:52 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 20:00:44 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP1_1_H
#define HTTP1_1_H

#include <algorithm>
#include <sstream>
#include <string>
#include "HTTPProtocol.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Utils.hpp"

class HTTP1_1 : public HTTPProtocol {
 public:
  HTTP1_1();
  virtual ~HTTP1_1();

  virtual HTTPRequest  parseRequest(const std::string& requestString);
  virtual HTTPResponse processRequest(const HTTPRequest& request);
};

#endif
