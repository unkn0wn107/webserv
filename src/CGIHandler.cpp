/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:05 by agaley            #+#    #+#             */
/*   Updated: 2024/06/07 01:33:50 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

Logger& CGIHandler::_log = Logger::getInstance();

CGIHandler::CGIHandler() {}

const std::pair<std::string, std::string> CGIHandler::_AVAILABLE_CGIS[] = {
    std::make_pair(".php", "/usr/bin/php-cgi"),
    std::make_pair(".py", "/usr/bin/python3")};

const int CGIHandler::_NUM_AVAILABLE_CGIS =
    sizeof(CGIHandler::_AVAILABLE_CGIS) /
    sizeof(std::pair<std::string, std::string>);

bool CGIHandler::isScript(const std::string& url) {
  if (_identifyRuntime(url).empty())
    return false;
  return true;
}

HTTPResponse* CGIHandler::processRequest(const HTTPRequest& request) {
  try {
    std::string runtime = _identifyRuntime(request.getURI());
    if (runtime.empty()) {
      throw RuntimeError("No suitable runtime found for script: " +
                         request.getURI());
    }
    std::string        cgiOutStr = _executeCGIScript(request, runtime);
    std::istringstream cgiOut(cgiOutStr);
    HTTPResponse*      response = new HTTPResponse();

    std::stringstream headers;
    std::string       line;
    while (std::getline(cgiOut, line) && line != "\r") {
      headers << line << "\n";
      std::size_t pos = line.find(":");
      _log.info("Header from CGI output: " + line.substr(0, pos) + ":" +
                line.substr(pos + 2));
      if (pos != std::string::npos) {
        response->addHeader(line.substr(0, pos), line.substr(pos + 2));
      }
    }

    std::stringstream body;
    body << cgiOutStr.substr(headers.str().size());

    response->setStatusCode(200);  // TODO: detect errors
    response->setBody(body.str());
    return response;
  } catch (const CGIHandler::Exception& e) {
    _log.error(e.what());
    return new HTTPResponse(500);  // Internal Server Error
  }
}

std::string CGIHandler::_identifyRuntime(const std::string& scriptPath) {
  size_t      lastDotPos = scriptPath.find_last_of('.');
  size_t      nextSlashPos = scriptPath.find_first_of("/", lastDotPos);
  std::string extension;

  if (lastDotPos != std::string::npos && nextSlashPos == std::string::npos) {
    extension = scriptPath.substr(lastDotPos);
  } else if (lastDotPos != std::string::npos &&
             nextSlashPos != std::string::npos && nextSlashPos > lastDotPos) {
    extension = scriptPath.substr(lastDotPos, nextSlashPos - lastDotPos);
  } else {
    extension = "";
  }

  for (int i = 0; i < CGIHandler::_NUM_AVAILABLE_CGIS; i++) {
    if (CGIHandler::_AVAILABLE_CGIS[i].first == extension) {
      return CGIHandler::_AVAILABLE_CGIS[i].second;
    }
  }
  return "";
}

std::string CGIHandler::_executeCGIScript(const HTTPRequest& request,
                                          const std::string& runtimePath) {
  int   pipefd[2];
  pid_t pid;

  _setupPipeAndFork(pipefd, pid);
  if (pid == 0) {  // Child process
    return _executeChildProcess(request, runtimePath, pipefd);
  } else {             // Parent process
    close(pipefd[1]);  // Close unused write end
    std::string output = _readFromPipe(&pipefd[0]);

    int status;
    waitpid(pid, &status, 0);  // Wait for child process to finish
    if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
      throw RuntimeError("CGI script exited with error");
    }
    return output;
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

std::string CGIHandler::_executeChildProcess(const HTTPRequest& request,
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

    pid_t pid2 = fork();
    if (pid2 == -1) {
      _log.error("Failed to fork process: " + std::string(strerror(errno)));
      exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {     // Child process 2
      close(pipefd[1]);  // Close unused write end
      write(pipefd[0], request.getBody().c_str(), request.getBody().length());
      close(pipefd[0]);  // Close write end of the pipe
      exit(EXIT_SUCCESS);
    } else {                   // Parent process
      waitpid(pid2, NULL, 0);  // Wait for child process 2 to finish
    }
  }

  std::string scriptUri = request.getURI();
  std::string pathInfo = "";
  size_t      scriptDot = scriptUri.find_last_of('.');
  size_t      nextSlash = scriptUri.substr(scriptDot).find_first_of("/");
  std::string scriptName;
  if (nextSlash != std::string::npos) {
    scriptName = scriptUri.substr(0, scriptDot + nextSlash);
  } else {
    scriptName = scriptUri;
  }

  // Prepare arguments
  std::vector<char*> argv;
  argv.push_back(const_cast<char*>(runtimePath.c_str()));  // Script path
  std::string scriptPath = "/var/www/html" + scriptName;
  argv.push_back(const_cast<char*>(scriptPath.c_str()));
  argv.push_back(NULL);

  // Execute CGI script
  execve(runtimePath.c_str(), &argv[0], _getEnvp(request));
  throw RuntimeError("Failed to execute CGI script: " +
                     std::string(strerror(errno)));
  exit(EXIT_FAILURE);
}

std::string CGIHandler::_readFromPipe(int pipefd[0]) {
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

char** CGIHandler::_getEnvp(const HTTPRequest& request) {
  std::vector<char*>                       env;
  const std::map<std::string, std::string> headers = request.getHeaders();

  std::string scriptUri = request.getURI();
  size_t      scriptDot = scriptUri.find_last_of('.');
  size_t      nextSlash = scriptUri.substr(scriptDot).find_first_of("/");
  std::string scriptName;
  std::string pathInfo;
  if (nextSlash != std::string::npos) {
    scriptName = scriptUri.substr(0, scriptDot + nextSlash);
    pathInfo = scriptUri.substr(scriptDot + nextSlash);
  } else {
    scriptName = scriptUri;
  }

  env.push_back(alloc_string("SCRIPT_NAME=" + scriptName));
  env.push_back(alloc_string("PATH_INFO=" + pathInfo));
  env.push_back(alloc_string("REQUEST_METHOD=" + request.getMethod()));
  env.push_back(alloc_string("QUERY_STRING=" + request.getQueryString()));
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

  char** envp = new char*[env.size()];
  for (size_t i = 0; i < env.size(); ++i) {
    envp[i] = env[i];
  }
  envp[env.size()] = NULL;

  return envp;
}
