/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:55 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:11:55 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class Handler {
 public:
  virtual ~Handler() {}
  virtual HTTPResponse handle(const HTTPRequest& request) = 0;
};

#endif
