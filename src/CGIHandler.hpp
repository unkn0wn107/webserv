/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:09 by agaley            #+#    #+#             */
/*   Updated: 2024/06/27 14:30:39 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_H
#define CGIHANDLER_H

#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <cctype>

#include "CacheHandler.hpp"
#include "Exception.hpp"
#include "FileManager.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Logger.hpp"

#define CGI_TIMEOUT_SEC 10

class CGIHandler {
 public:
  /**
   * Check if url has an executable file extension.
   * @param request The HTTP request object.
   * @return true if the URL ends with a registered CGI script extension.
   */
  static bool isScript(const HTTPRequest& request);

  /**
   * Handles the CGI request and generates an HTTP response.
   * @param request The HTTP request object.
   * @return HTTPResponse object containing the response from the CGI script.
   */
  static HTTPResponse* handleCGIRequest(HTTPRequest& request);

  class NoRuntimeError : public Exception {
   public:
    NoRuntimeError(const std::string& message) : Exception(message) {}
  };

  class RuntimeError : public Exception {
   public:
    RuntimeError(const std::string& message) : Exception(message) {}
  };

  class ExecutorError : public Exception {
   public:
    ExecutorError(const std::string& message) : Exception(message) {}
  };

  class CGIDisabled : public Exception {
   public:
    CGIDisabled(const std::string& message) : Exception(message) {}
  };

  class CGINotFound : public Exception {
   public:
    CGINotFound(const std::string& message) : Exception(message) {}
  };

  class CGINotExecutable : public Exception {
   public:
    CGINotExecutable(const std::string& message) : Exception(message) {}
  };

  class ScriptNotFound : public Exception {
   public:
    ScriptNotFound(const std::string& message) : Exception(message) {}
  };

  class ScriptNotExecutable : public Exception {
   public:
    ScriptNotExecutable(const std::string& message) : Exception(message) {}
  };

  class PipeFailure : public Exception {
   public:
    PipeFailure(const std::string& message) : Exception(message) {}
  };

  class ForkFailure : public Exception {
   public:
    ForkFailure(const std::string& message) : Exception(message) {}
  };

  class TimeoutException : public Exception {
   public:
    TimeoutException(const std::string& message) : Exception(message) {}
  };

 private:
  CGIHandler();

  static Logger&       _log;
  static CacheHandler& _cacheHandler;

  static const std::pair<std::string, std::string> _AVAILABLE_CGIS[];
  static const int                                 _NUM_AVAILABLE_CGIS;

  /**
   * Identifies the runtime environment based on the script file extension.
   * @param request The HTTP request object.
   * @return String representing the runtime to be used.
   */
  static const std::string _identifyRuntime(const HTTPRequest& request);

  /**
   * Checks if the processing of the request is possible.
   * @param request The HTTP request object.
   * @param runtime The runtime to be used.
   */
  static void _checkIfProcessingPossible(const HTTPRequest& request,
                                         const std::string& runtime);

  /**
   * Processes the CGI request and generates an HTTP response.
   * @param request The HTTP request object containing the CGI request details.
   * @return HTTPResponse object containing the response from the CGI script.
   */
  static HTTPResponse* _processRequest(const HTTPRequest& request);

  /**
   * Creates a null-terminated array of CGI environment variables from an HTTP
   * request. Includes variables like SCRIPT_NAME, PATH_INFO, REQUEST_METHOD,
   * QUERY_STRING, REMOTE_HOST, CONTENT_LENGTH, and headers prefixed 'HTTP_'.
   * @param request The HTTP request object.
   * @return Array of environment variable strings.
   */
  static std::vector<char*> _getEnvp(const HTTPRequest& request);

  /**
   * Generates a list of arguments for the CGI script based on the HTTP request.
   * @param request The HTTP request object.
   * @return A vector of strings, each representing an argument for the CGI
   * script.
   */
  static std::vector<char*> _getArgv(const HTTPRequest& request);

  /**
   * Executes the parent process logic for CGI script execution.
   * @param pipefd Array holding file descriptors for the pipe.
   * @param pid The process ID of the forked process.
   * @param argv A vector of strings, each representing an argument for the CGI
   * script.
   * @param envp A vector of strings, each representing an environment variable
   * for the CGI script.
   * @return HTTPResponse object containing the response from the CGI script.
   */
  static HTTPResponse* _executeParentProcess(int pipefd[2], pid_t pid);

  /**
   * Executes the child process logic for CGI script execution.
   * @param request The HTTP request object.
   * @param pipefd Array holding file descriptors for the pipe.
   * @param argv A vector of strings, each representing an argument for the CGI
   * script.
   * @param envp A vector of strings, each representing an environment variable
   * for the CGI script.
   * @return String containing the output from the CGI script.
   */
  static void _executeChildProcess(const HTTPRequest& request,
                                   int                pipefd[2],
                                   std::vector<char*> argv,
                                   std::vector<char*> envp);
};

#endif
