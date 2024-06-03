/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:05 by agaley            #+#    #+#             */
/*   Updated: 2024/06/04 01:26:31 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

CGIHandler::CGIHandler() {}

const std::pair<std::string, std::string> CGIHandler::_AVAILABLE_CGIS[] = {
    std::make_pair(".php", "/usr/bin/php-cgi"),
    std::make_pair(".py", "/usr/bin/python")};

const int CGIHandler::_NUM_AVAILABLE_CGIS =
    sizeof(CGIHandler::_AVAILABLE_CGIS) /
    sizeof(std::pair<std::string, std::string>);

bool CGIHandler::isScript(const std::string& url) {
  size_t      lastDotPos = url.find_last_of('.');
  std::string extension;
  if (lastDotPos != std::string::npos)
    extension = url.substr(lastDotPos);
  else
    extension = "";

  for (int i = 0; i < CGIHandler::_NUM_AVAILABLE_CGIS; i++) {
    if (CGIHandler::_AVAILABLE_CGIS[i].first == extension) {
      return true;
    }
  }
  return false;
}

HTTPResponse* CGIHandler::processRequest(const HTTPRequest& request) {
  std::string runtime = _identifyRuntime(request.getURI());
  if (runtime.empty()) {
    std::cerr << "No suitable runtime found for script: " << request.getURI()
              << std::endl;
    return new HTTPResponse(500);
  }
  HTTPResponse* response = new HTTPResponse(200);
  response->setBody(_executeCGIScript(request, runtime));
  return response;
}

std::string CGIHandler::_identifyRuntime(const std::string& scriptPath) {
  std::string extension = scriptPath.substr(scriptPath.find_last_of('.'));
  for (int i = 0; i < CGIHandler::_NUM_AVAILABLE_CGIS; i++) {
    if (CGIHandler::_AVAILABLE_CGIS[i].first == extension) {
      return CGIHandler::_AVAILABLE_CGIS[i].second;
    }
  }
  return "";
}

std::string CGIHandler::_executeCGIScript(const HTTPRequest& request,
                                          const std::string& scriptPath) {
  Logger log = Logger::getInstance();

  int pipefd[2];
  if (pipe(pipefd) == -1) {
    log.error("Failed to create pipe: " + std::string(strerror(errno)));
    return "";
  }

  pid_t pid = 0;
  pid = fork();
  if (pid == -1) {
    log.error("Failed to fork process: " + std::string(strerror(errno)));
    close(pipefd[0]);
    close(pipefd[1]);
    return "";
  }

  if (pid == 0) {                                // Child process
    close(pipefd[0]);                            // Close unused read end
    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {  // Redirect stdout to pipe
      log.error("Failed to redirect stdout to pipe: " +
                std::string(strerror(errno)));
      exit(EXIT_FAILURE);
    }
    close(pipefd[1]);  // Close write end of the pipe

    // Prepare arguments
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(scriptPath.c_str()));  // Script path
    log.info("Script path: " + scriptPath);
    log.info("Request URI: " + request.getURI());
    argv.push_back(const_cast<char*>(
        ("/home/user/webserv/site/" + request.getURI()).c_str()));
    argv.push_back(NULL);

    // Execute CGI script
    execve(scriptPath.c_str(), &argv[0], _getEnvp(request));
    log.error("Failed to execute CGI script: " + std::string(strerror(errno)));
    exit(EXIT_FAILURE);
  } else {             // Parent process
    close(pipefd[1]);  // Close unused write end

    std::ostringstream oss;
    char               buffer[1024];
    ssize_t            bytesRead;
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
      oss.write(buffer, bytesRead);
    }
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);  // Wait for child process to finish

    if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS) {
      return oss.str();
    } else {
      log.error("CGI script exited with error");
      return "";
    }
  }
}

char** CGIHandler::_getEnvp(const HTTPRequest& request) {
  std::vector<char*>                       env;
  const std::map<std::string, std::string> headers = request.getHeaders();

  env.push_back(const_cast<char*>(("SCRIPT_NAME=" + request.getURI()).c_str()));
  env.push_back(
      const_cast<char*>(("REQUEST_METHOD=" + request.getMethod()).c_str()));
  env.push_back(
      const_cast<char*>(("QUERY_STRING=" + request.getQueryString()).c_str()));
  env.push_back(const_cast<char*>("REMOTE_HOST=localhost"));
  env.push_back(const_cast<char*>(
      ("CONTENT_LENGTH=" + Utils::to_string(request.getBody().length()))
          .c_str()));

  for (std::map<std::string, std::string>::const_iterator hd = headers.begin();
       hd != headers.end(); ++hd) {
    std::string envName = "HTTP_" + hd->first;
    std::replace(envName.begin(), envName.end(), '-', '_');
    std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
    std::string envLine = envName + "=" + hd->second;
    env.push_back(const_cast<char*>(envLine.c_str()));
  }

  env.push_back(NULL);

  char** envp = new char*[env.size()];
  for (size_t i = 0; i < env.size(); ++i) {
    envp[i] = env[i];
  }

  return envp;
}
