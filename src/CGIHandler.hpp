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
  /**
   * A map to store the association between CGI script names and their binaries
   */
  static std::map<std::string, std::string> availableCGIs;

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
  std::string runScript(const std::string& scriptPath,
                        const HTTPRequest& request) const;

  std::string executeCGIScript(const std::string& scriptPath,
                               const HTTPRequest& request) const;
};

#endif
