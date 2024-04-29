#ifndef CGIHANDLER_H
#define CGIHANDLER_H

#include <string>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

/**
 * Class CGIHandler is responsible for handling CGI scripts execution.
 * It identifies the appropriate runtime for the script based on its extension,
 * executes the script, and captures its output to form an HTTP response.
 */
class CGIHandler {
 public:
  /**
   * Constructor for CGIHandler.
   */
  CGIHandler();

  /**
   * Processes the CGI request and generates an HTTP response.
   * @param request The HTTP request object containing the CGI request details.
   * @return HTTPResponse object containing the response from the CGI script.
   */
  HTTPResponse processRequest(const HTTPRequest& request);

 private:
  /**
   * Identifies the runtime environment based on the script file extension.
   * @param scriptPath The path to the CGI script.
   * @return String representing the runtime to be used.
   */
  std::string identifyRuntime(const std::string& scriptPath);

  /**
   * Executes the CGI script using the identified runtime.
   * @param scriptPath The path to the CGI script.
   * @param request The HTTP request object for passing to the CGI.
   * @return String containing the output from the CGI script.
   */
  std::string runScript(const std::string& scriptPath,
                        const HTTPRequest& request);
};

#endif  // CGIHANDLER_H
