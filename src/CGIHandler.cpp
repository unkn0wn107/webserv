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

// HTTPResponse* CGIHandler::processRequest(const HTTPRequest& request) {
//   if (!request.getConfig()->cgi)
//     throw CGIDisabled("Execution forbidden by config: " + request.getURI());

//   std::string runtime = _identifyRuntime(request);
//   if (runtime.empty())
//     throw NoRuntimeError("No suitable runtime found for script: " + request.getURI());

//   if (!FileManager::doesFileExists(runtime))
//     throw CGINotFound("CGI not found: " + runtime);
//   if (!FileManager::isFileExecutable(runtime))
//     throw CGINotExecutable("CGI not executable: " + runtime);

//   std::string cgiOutStr = _executeCGIScript(request, runtime);
//   std::size_t headerEndPos = cgiOutStr.find("\r\n\r\n");
//   std::string headerPart = cgiOutStr.substr(0, headerEndPos);
//   std::string bodyContent = cgiOutStr.substr(headerEndPos + 4); // +4 to skip the "\r\n\r\n"

//   std::map<std::string, std::string> headers;
//   std::istringstream headerStream(headerPart);
//   std::string line;

//   while (std::getline(headerStream, line)) {
//     std::size_t colonPos = line.find(':');
//     if (colonPos != std::string::npos) {
//       std::string key = line.substr(0, colonPos);
//       std::string value = line.substr(colonPos + 2); // +2 to skip ": "
//       headers[key] = value;
//     }
//   }

//   // Assurez-vous que l'en-tête Content-Length est correct
//   headers["Content-Length"] = Utils::to_string(bodyContent.size());

//   return new HTTPResponse(HTTPResponse::OK, headers, bodyContent);
// }
HTTPResponse* CGIHandler::processRequest(const HTTPRequest& request) {
    if (!request.getConfig()->cgi)
        throw CGIDisabled("Execution forbidden by config: " + request.getURI());

    std::string runtime = _identifyRuntime(request);
    if (runtime.empty())
        throw NoRuntimeError("No suitable runtime found for script: " + request.getURI());

    if (!FileManager::doesFileExists(runtime))
        throw CGINotFound("CGI not found: " + runtime);
    if (!FileManager::isFileExecutable(runtime))
        throw CGINotExecutable("CGI not executable: " + runtime);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        throw RuntimeError("Failed to create pipe");
    }

    pid_t pid = fork();
    if (pid == -1) {
        throw RuntimeError("Failed to fork process");
    } else if (pid == 0) {  // Child process
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
        std::string postData = request.getBody();  // Assuming getBody() fetches the raw POST data
        write(postPipe[1], postData.c_str(), postData.size());
        close(postPipe[1]);  // Close after writing

        // Execute CGI script - TODO : remove stderr
        char** envp = _getEnvp(request);
        std::cerr << "Environment variables for CGI script:" << std::endl;
        for (char** env = envp; *env != NULL; ++env) {
          std::cerr << *env << std::endl;
        }
        std::vector<char*> argv = _getArgv(request);
        execve(runtime.c_str(), &argv[0], envp);
        perror("execve failed");
        exit(EXIT_FAILURE);  // execve only returns on error
    } else {  // Parent process
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
            kill(pid, SIGKILL);  // Kill the child process
            throw TimeoutException("CGI script execution timed out");
        } else {
        std::string output;
        char buffer[1024];
        ssize_t count;
        while ((count = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, count);
        }
        close(pipefd[0]);

        int status;
        // _log.info("#### Waiting for pid");
        waitpid(pid, &status, 0);  // Wait for the child process to finish
        // _log.info("#### DONE for pid");

        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            throw RuntimeError("CGI script failed");
        }

        // Parse output to create HTTPResponse
        std::size_t headerEndPos = output.find("\r\n\r\n");
        std::string headerPart = output.substr(0, headerEndPos);
        std::string bodyContent = output.substr(headerEndPos + 4); // +4 to skip the "\r\n\r\n"

        std::map<std::string, std::string> headers;
        std::istringstream headerStream(headerPart);
        std::string line;
        while (std::getline(headerStream, line)) {
            std::size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 2); // +2 to skip ": "
                headers[key] = value;
            }
        }

        headers["Content-Length"] = Utils::to_string(bodyContent.size());

        return new HTTPResponse(HTTPResponse::OK, headers, bodyContent);
    }
    }
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

const std::string CGIHandler::_executeCGIScript(
    const HTTPRequest& request,
    const std::string& runtimePath) {
  int   pipefd[2];
  pid_t pid;

  _setupPipeAndFork(pipefd, pid);
  if (pid == 0) {  // Child process
    return _executeChildProcess(request, runtimePath, pipefd);
  } else {             // Parent process
    close(pipefd[1]);  // Close unused write end

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(pipefd[0], &read_fds);

    struct timeval timeout;
    timeout.tv_sec = CGI_TIMEOUT_SEC;
    timeout.tv_usec = 0;

    int retval = select(pipefd[0] + 1, &read_fds, NULL, NULL, &timeout);
    if (retval == -1) {
      // close(pipefd[0]);
      throw RuntimeError("Select error: " + std::string(strerror(errno)));
    } else if (retval == 0) {
      // close(pipefd[0]);
      kill(pid, SIGKILL);
      throw TimeoutException("CGI script execution timed out : " +
                             Utils::to_string(CGI_TIMEOUT_SEC) + "s");
    } else {
      std::string output = _readFromPipe(&pipefd[0]);

      int status;
      std::cerr << "#### waiting pid1" << std::endl;
      waitpid(pid, &status, 0);  // Wait for child process to finish
      std::cerr << "#### Finished waiting pid1" << std::endl;
      if (WIFEXITED(status)) {
        int exitStatus = WEXITSTATUS(status);
        if (exitStatus != EXIT_SUCCESS) {
          throw RuntimeError("CGI script exited with error, status code: " +
                             Utils::to_string(exitStatus));
        }
      } else if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        throw RuntimeError("CGI script killed by signal: " +
                           Utils::to_string(signal));
      }
      return output;
    }
  }
}

void CGIHandler::_setupPipeAndFork(int pipefd[2], pid_t& pid) {
  if (pipe(pipefd) == -1) {
    throw PipeFailure("Failed to create pipe: " + std::string(strerror(errno)));
  }

  pid = fork();
  if (pid == -1) {
    close(pipefd[0]);
    close(pipefd[1]);
    throw ForkFailure("Failed to fork process: " +
                      std::string(strerror(errno)));
  }
}

const std::string CGIHandler::_executeChildProcess(
    const HTTPRequest& request,
    const std::string& runtimePath,
    int                pipefd[2]) {
  if (dup2(pipefd[1], STDOUT_FILENO) == -1) {  // Redirect stdout to pipe
    _log.error("Failed to redirect stdout to pipe: " +
               std::string(strerror(errno)));
    exit(EXIT_FAILURE);
  }
  close(pipefd[1]);  // Close write end of the pipe

  if (request.getMethod() == "POST") {
    if (dup2(pipefd[0], STDIN_FILENO) == -1) {  // Redirect stdin to pipe
      _log.error("Failed to redirect stdin to pipe: " +
                 std::string(strerror(errno)));
      exit(EXIT_FAILURE);
    }
    close(pipefd[0]);  // Close read end of the pipe

    // if (pipe(pipefd) == -1) {
    //   throw PipeFailure("Failed to create pipe: " + std::string(strerror(errno)));
    // }

    pid_t pid2 = fork();
    if (pid2 == -1) {
      _log.error("Failed to fork process: " + std::string(strerror(errno)));
      exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {     // Child process 2
      close(pipefd[1]);  // Close unused write end
      if (!request.getBody().empty())
        write(pipefd[0], request.getBody().c_str(), request.getBody().length());
      close(pipefd[0]);  // Close write end of the pipe
      exit(EXIT_SUCCESS);
    } else {                   // Parent process
     std::cerr << "#### waiting pid2" << std::endl;
      waitpid(pid2, NULL, 0);  // Wait for child process 2 to finish
      std::cerr << "#### finished waiting pid2" << std::endl;
    }
  }

  // Prepare arguments
  std::vector<char*> argv;
  argv.push_back(const_cast<char*>(runtimePath.c_str()));
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

  // Execute CGI script - TODO : remove stderr
  char** envp = _getEnvp(request);
  std::cerr << "Environment variables for CGI script:" << std::endl;
  for (char** env = envp; *env != NULL; ++env) {
    std::cerr << *env << std::endl;
  }
  execve(runtimePath.c_str(), &argv[0], envp);
  throw RuntimeError("Failed to execute CGI script: " +
                     std::string(strerror(errno)));
  exit(EXIT_FAILURE);
}

const std::string CGIHandler::_readFromPipe(int pipefd[0]) {
  std::ostringstream oss;
  char               buffer[1024];
  ssize_t            bytesRead;
  while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
    oss.write(buffer, bytesRead);
  }
  close(pipefd[0]);
  return oss.str();
}

char* alloc_string(const std::string& str) {
  char* result = new char[str.length() + 1];
  std::strcpy(result, str.c_str());
  return result;
}

std::vector<char*> CGIHandler::_getArgv(const HTTPRequest& request) {
  std::vector<char*> argv;
  std::string runtime = _identifyRuntime(request);
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

char** CGIHandler::_getEnvp(const HTTPRequest& request) {
  std::vector<char*>                       env;
  const std::map<std::string, std::string> headers = request.getHeaders();
  const URI::Components uriComponents = request.getURIComponents();

  env.push_back(alloc_string("REDIRECT_STATUS=200"));  // For php-cgi at least
  env.push_back(alloc_string("SCRIPT_FILENAME=" + request.getConfig()->root +
                             uriComponents.scriptName));
  env.push_back(alloc_string("SCRIPT_NAME=" + uriComponents.scriptName));
  env.push_back(alloc_string("PATH_INFO=" + uriComponents.pathInfo));
  env.push_back(alloc_string("REQUEST_METHOD=" + request.getMethod()));
  env.push_back(alloc_string("QUERY_STRING=" + uriComponents.queryString));
  env.push_back(alloc_string("REMOTE_HOST=localhost"));
  env.push_back(alloc_string(
      ("CONTENT_LENGTH=" + Utils::to_string(request.getBody().length()))
          .c_str()));

  for (std::map<std::string, std::string>::const_iterator hd = headers.begin();
       hd != headers.end(); ++hd) {
    std::string envName = "HTTP_" + hd->first;
    std::replace(envName.begin(), envName.end(), '-', '_');
    std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
    std::string envLine = envName + "=" + hd->second;
    env.push_back(alloc_string(envLine));
  }

  env.push_back(NULL);

  char** envp = new char*[env.size() + 1];
  size_t i;
  for (i = 0; i < env.size(); ++i) {
    envp[i] = env[i];
  }
  envp[i] = NULL;

  return envp;
}
