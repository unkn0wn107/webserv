/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileHandler.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:41 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 19:47:13 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "Handler.hpp"

class HTTPRequest;
class HTTPResponse;

class FileHandler : public Handler {
 private:
  FileHandler();
  ~FileHandler();

 public:
  static HTTPResponse processRequest(const HTTPRequest& request, ServerConfig& config);
};

#endif
