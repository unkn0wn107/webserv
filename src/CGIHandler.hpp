/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:09 by agaley            #+#    #+#             */
/*   Updated: 2024/06/28 13:43:23 by agaley           ###   ########.fr       */
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
#include "ConnectionHandler.hpp"
#include "Common.hpp"
#include "Logger.hpp"
#include <sys/epoll.h>
#define CGI_TIMEOUT_SEC 10
// enum CGIStatus { READING, EXECUTING, SENDING, CLOSED };


class ConnectionHandler;

class CGIHandler {
  public:
    CGIHandler(HTTPRequest& request,
    HTTPResponse& response, int epollSocket, LocationConfig& location, ConnectionHandler* connectionHandler);
    ~CGIHandler();

  int   getCgifd();

  /**
   * Check if url has an executable file extension.
   * @param request The HTTP request object.
   * @return true if the URL ends with a registered CGI script extension.
   */
  static bool isScript(const HTTPRequest& request, LocationConfig& location);

  /**
   * Handles the CGI request and generates an HTTP response.
   * @return HTTPResponse object containing the response from the CGI script.
   */
  int handleCGIRequest();

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

  class MutexFailure : public Exception {
    public:
      MutexFailure(const std::string& message) : Exception(message) {}
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
  static Logger&       _log;
  static CacheHandler& _cacheHandler;

  int _epollSocket;
  ConnectionHandler* _connectionHandler;
  HTTPRequest&  _request;
  HTTPResponse& _response;
  LocationConfig& _location;
  std::string   _processOutput;
  size_t        _processOutputSize;
  std::string   _runtime;
  std::string   _root;
  std::string   _index;
  bool          _cgi;
  bool          _done;
  pthread_mutex_t _mutex;

  std::vector<char*> _argv;
  std::vector<char*> _envp;

  int   _inpipefd[2];
  int   _outpipefd[2];
  pid_t _pid;

  static const std::pair<std::string, std::string> _AVAILABLE_CGIS[];
  static const int                                 _NUM_AVAILABLE_CGIS;

  /**
   * Identifies the runtime environment based on the script file extension.
   * @param request The HTTP request object.
   * @return String representing the runtime to be used.
   */
  static const   std::string _identifyRuntime(const HTTPRequest& request, LocationConfig& location);

  /**
   * Checks if the processing of the request is possible.
   * @param request The HTTP request object.
   * @param runtime The runtime to be used.
   */
  void _checkIfProcessingPossible();

  /**
   * Creates a null-terminated array of CGI environment variables from an HTTP
   * request. Includes variables like SCRIPT_NAME, PATH_INFO, REQUEST_METHOD,
   * QUERY_STRING, REMOTE_HOST, CONTENT_LENGTH, and headers prefixed 'HTTP_'.
   * @param request The HTTP request object.
   * @return Array of environment variable strings.
   */
  static std::vector<char*> _getEnvp(const HTTPRequest& request, LocationConfig& location);

  /**
   * Generates a list of arguments for the CGI script based on the HTTP request.
   * @param request The HTTP request object.
   * @return A vector of strings, each representing an argument for the CGI
   * script.
   */
  static std::vector<char*> _getArgv(const HTTPRequest& request, LocationConfig& location);

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
  int   _executeParentProcess();
  int   _processRequest();
  void  _runScript();
};

#endif
