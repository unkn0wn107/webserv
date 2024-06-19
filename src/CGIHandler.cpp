/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:05 by agaley            #+#    #+#             */
/*   Updated: 2024/06/10 00:45:22 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

Logger& CGIHandler::_log = Logger::getInstance();

CGIHandler::CGIHandler() {}

const std::pair<std::string, std::string> CGIHandler::_AVAILABLE_CGIS[] = {
    std::make_pair(".php", "/usr/bin/php-cgi"),
    std::make_pair(".py", "/usr/bin/python3"),
    std::make_pair(".bla", "/usr/bin/ubuntu_cgi_tester")};

const int CGIHandler::_NUM_AVAILABLE_CGIS =
    sizeof(CGIHandler::_AVAILABLE_CGIS) /
    sizeof(std::pair<std::string, std::string>);

bool CGIHandler::isScript(const HTTPRequest& request) {
  if (_identifyRuntime(request).empty())
    return false;
  return true;
}

void CGIHandler::_checkIfProcessingPossible(const HTTPRequest& request,
                                            const std::string& runtime) {
  if (!request.getConfig()->cgi)
    throw CGIDisabled("Execution forbidden by config: " + request.getURI());
  if (runtime.empty())
    throw NoRuntimeError("No suitable runtime found for script: " +
                         request.getURI());
  if (!FileManager::doesFileExists(runtime))
    throw CGINotFound("CGI not found: " + runtime);
  if (!FileManager::isFileExecutable(runtime))
    throw CGINotExecutable("CGI not executable: " + runtime);
}

HTTPResponse* CGIHandler::processRequest(const HTTPRequest& request) {
  std::string runtime = _identifyRuntime(request);
  _checkIfProcessingPossible(request, runtime);

  int pipefd[2];
  if (pipe(pipefd) == -1)
    throw RuntimeError("Failed to create pipe");

  pid_t pid = fork();
  if (pid == -1)
    throw RuntimeError("Failed to fork process");
  else if (pid == 0)
    _executeChildProcess(request, runtime, pipefd);
  else
    return _executeParentProcess(pipefd, pid);
  throw RuntimeError("CGI script fatal : out of pid branches !!");
}

const std::string CGIHandler::_identifyRuntime(const HTTPRequest& request) {
  const URI::Components uriComponents = request.getURIComponents();
  _log.info("Identifying extension for script: " + uriComponents.extension);
  for (int i = 0; i < CGIHandler::_NUM_AVAILABLE_CGIS; i++) {
    if (CGIHandler::_AVAILABLE_CGIS[i].first == uriComponents.extension) {
      return CGIHandler::_AVAILABLE_CGIS[i].second;
    }
  }
  return "";
}

HTTPResponse* CGIHandler::_executeParentProcess(int pipefd[2], pid_t pid) {
  close(pipefd[1]);  // Close the write end of the pipe

  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(pipefd[0], &read_fds);
  struct timeval timeout;
  timeout.tv_sec = CGI_TIMEOUT_SEC;
  timeout.tv_usec = 0;

  int retval = select(pipefd[0] + 1, &read_fds, NULL, NULL, &timeout);
  if (retval == -1) {
    throw RuntimeError("Select error");
  } else if (retval == 0) {
    kill(pid, SIGKILL);
    throw TimeoutException("CGI script execution timed out");
  } else {
    std::string output;
    char        buffer[1024];
    ssize_t     count;
    while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
      output.append(buffer, count);
    }
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      throw RuntimeError("CGI script failed");
    }

    // Parse output to create HTTPResponse
    std::size_t headerEndPos = output.find("\r\n\r\n");
    std::string headerPart = output.substr(0, headerEndPos);
    std::string bodyContent =
        output.substr(headerEndPos + 4);  // +4 to skip the "\r\n\r\n"

    std::map<std::string, std::string> headers;
    std::istringstream                 headerStream(headerPart);
    std::string                        line;
    while (std::getline(headerStream, line)) {
      std::size_t colonPos = line.find(':');
      if (colonPos != std::string::npos) {
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 2);  // +2 to skip ": "
        headers[key] = value;
      }
    }
    headers["Content-Length"] = Utils::to_string(bodyContent.size());
    return new HTTPResponse(HTTPResponse::OK, headers, bodyContent);
  }
  throw RuntimeError("CGI script failed : out of timeout loop");
}

void CGIHandler::_executeChildProcess(const HTTPRequest& request,
                                      const std::string& runtimePath,
                                      int                pipefd[2]) {
  close(pipefd[0]);  // Close the read end of the pipe

  // Redirect stdout to pipe
  if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
    perror("dup2 failed");
    exit(EXIT_FAILURE);
  }
  close(pipefd[1]);

  // Handling POST data by redirecting stdin to a pipe
  int postPipe[2];
  if (pipe(postPipe) == -1) {
    perror("pipe failed");
    exit(EXIT_FAILURE);
  }

  if (dup2(postPipe[0], STDIN_FILENO) == -1) {
    perror("dup2 failed");
    exit(EXIT_FAILURE);
  }
  close(postPipe[0]);  // Close the read end of the pipe

  // Write POST data to postPipe[1] so it can be read from postPipe[0]
  std::string postData =
      request.getBody();  // Assuming getBody() fetches the raw POST data
  write(postPipe[1], postData.c_str(), postData.size());
  close(postPipe[1]);  // Close after writing

  // Execute CGI script - TODO : remove stderr
  char** envp = _getEnvp(request);
  std::cerr << "Environment variables for CGI script:" << std::endl;
  for (char** env = envp; *env != NULL; ++env) {
    std::cerr << *env << std::endl;
  }
  std::vector<char*> argv = _getArgv(request);
  execve(runtimePath.c_str(), &argv[0], envp);
  perror("execve failed");
  exit(EXIT_FAILURE);  // execve only returns on error
}

std::vector<char*> CGIHandler::_getArgv(const HTTPRequest& request) {
  std::vector<char*> argv;
  std::string        runtime = _identifyRuntime(request);
  argv.push_back(const_cast<char*>(runtime.c_str()));
  std::string scriptPath =
      request.getConfig()->root + request.getURIComponents().scriptName;
  if (!FileManager::doesFileExists(scriptPath))
    throw ScriptNotFound("Script not found: " + scriptPath);
  if (!FileManager::isFileExecutable(scriptPath))
    throw ScriptNotExecutable("Script not executable: " + scriptPath);
  argv.push_back(const_cast<char*>(scriptPath.c_str()));
  argv.push_back(NULL);
  if (argv.empty()) {
    std::cerr << "Argument vector is empty, cannot execute CGI script."
              << std::endl;
  } else {
    std::string argsDebugInfo = "Executing CGI script with arguments: ";
    for (size_t i = 0; i < argv.size(); ++i) {
      if (argv[i] != NULL) {
        argsDebugInfo += std::string(argv[i]) + " ";
      }
    }
    std::cerr << argsDebugInfo << std::endl;
  }
  return argv;
}

char* alloc_string(const std::string& str) {
  char* result = new char[str.length() + 1];
  std::strcpy(result, str.c_str());
  return result;
}

char** CGIHandler::_getEnvp(const HTTPRequest& request) {
  const std::map<std::string, std::string> headers = request.getHeaders();
  const URI::Components uriComponents = request.getURIComponents();

  char** envp = new char*[headers.size() + 8 + 1];  // 8 env variables + NULL

  envp[0] = alloc_string("REDIRECT_STATUS=200");  // For php-cgi at least
  envp[1] = alloc_string("SCRIPT_FILENAME=" + request.getConfig()->root +
                         uriComponents.scriptName);
  envp[2] = alloc_string("SCRIPT_NAME=" + uriComponents.scriptName);
  envp[3] = alloc_string("PATH_INFO=" + uriComponents.pathInfo);
  envp[4] = alloc_string("REQUEST_METHOD=" + request.getMethod());
  envp[5] = alloc_string("QUERY_STRING=" + uriComponents.queryString);
  envp[6] = alloc_string("REMOTE_HOST=localhost");
  envp[7] = alloc_string(
      ("CONTENT_LENGTH=" + Utils::to_string(request.getBody().length()))
          .c_str());

  size_t i = 0;
  for (std::map<std::string, std::string>::const_iterator hd = headers.begin();
       hd != headers.end(); ++hd) {
    std::string envName = "HTTP_" + hd->first;
    std::replace(envName.begin(), envName.end(), '-', '_');
    std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
    std::string envLine = envName + "=" + hd->second;
    envp[8 + i++] = alloc_string(envLine);
  }
  envp[8 + i] = NULL;
  return envp;
}
