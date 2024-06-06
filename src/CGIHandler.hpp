/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:09 by agaley            #+#    #+#             */
/*   Updated: 2024/06/07 01:03:57 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_H
#define CGIHANDLER_H

#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"

class CGIHandler {
 public:
  CGIHandler();

  static bool isScript(const std::string& url);
  /**
   * Processes the CGI request and generates an HTTP response.
   * @param request The HTTP request object containing the CGI request details.
   * @return HTTPResponse object containing the response from the CGI script.
   */
  static HTTPResponse* processRequest(const HTTPRequest& request);

  class Exception : public std::exception {
   protected:
    std::string _message;

   public:
    Exception(const std::string& message) : _message(message) {}
    virtual ~Exception() throw() {}
    virtual const char* what() const throw() { return _message.c_str(); }
  };

  class RuntimeError : public Exception {
   public:
    RuntimeError(const std::string& message) : Exception(message) {}
  };

  class PipeFailure : public Exception {
   public:
    PipeFailure(const std::string& message) : Exception(message) {}
  };

  class ForkFailure : public Exception {
   public:
    ForkFailure(const std::string& message) : Exception(message) {}
  };

 private:
  static Logger& _log;

  static const std::pair<std::string, std::string> _AVAILABLE_CGIS[];
  static const int                                 _NUM_AVAILABLE_CGIS;

  static char** _getEnvp(const HTTPRequest& request);
  /**
   * Identifies the runtime environment based on the script file extension.
   * @param scriptPath The path to the CGI script.
   * @return String representing the runtime to be used.
   */
  static std::string _identifyRuntime(const std::string& scriptPath);

  /**
   * Executes the CGI script using the identified runtime.
   * @param request The HTTP request object for passing to the CGI.
   * @param runtimePath The path to the CGI runtime.
   * @return String containing the output from the CGI script.
   */
  static std::string _executeCGIScript(const HTTPRequest& request,
                                       const std::string& runtimePath);
};

#endif
