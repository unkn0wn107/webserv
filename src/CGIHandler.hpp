/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:09 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 18:59:42 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_H
#define CGIHANDLER_H

#include <sys/wait.h>
#include <unistd.h>
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

class CGIHandler {
 public:
  CGIHandler();

  static bool isScript(const std::string& url);
  /**
   * Processes the CGI request and generates an HTTP response.
   * @param request The HTTP request object containing the CGI request details.
   * @return HTTPResponse object containing the response from the CGI script.
   */
  static HTTPResponse processRequest(const HTTPRequest& request);

 private:
  static const std::pair<std::string, std::string> _AVAILABLE_CGIS[];
  static const int                                 _NUM_AVAILABLE_CGIS;

  /**
   * Identifies the runtime environment based on the script file extension.
   * @param scriptPath The path to the CGI script.
   * @return String representing the runtime to be used.
   */
  std::string identifyRuntime(const std::string& scriptPath) const;

  /**
   * Executes the CGI script using the identified runtime.
   * @param scriptPath The path to the CGI script.
   * @param request The HTTP request object for passing to the CGI.
   * @return String containing the output from the CGI script.
   */
  std::string runScript(const std::string& scriptPath) const;
  std::string executeCGIScript(const std::string& scriptPath) const;
};

#endif
