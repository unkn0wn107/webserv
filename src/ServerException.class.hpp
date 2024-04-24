/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.class.hpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:13:20 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 17:05:11 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <exception>
#include <string>
#include "Logger.class.hpp"

class ServerException : public std::exception{
 private:
  Logger& _log;
  std::string _message;

 public:
  ServerException();
  ServerException(const std::string& msg);
  ServerException(const ServerException& other);
  ServerException& operator=(const ServerException& other);
  virtual ~ServerException() throw();

  virtual const char* what() const throw();
};
