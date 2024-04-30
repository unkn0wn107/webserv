/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:05 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:41:25 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

CGIHandler::CGIHandler() {}

bool CGIHandler::isScript(const std::string& url) {
  std::string extension = url.substr(url.find_last_of('.'));
  return _AVAILABLE_CGIS.find(extension) != _AVAILABLE_CGIS.end();
}

std::string CGIHandler::identifyRuntime(const std::string& scriptPath) const {
  std::string extension = scriptPath.substr(scriptPath.find_last_of('.'));
  std::map<std::string, std::string>::const_iterator it =
      _AVAILABLE_CGIS.find(extension);
  if (it != _AVAILABLE_CGIS.end()) {
    return it->second;
  }
  return "";
}

std::string CGIHandler::runScript(const std::string& scriptPath) const {
  std::string runtime = identifyRuntime(scriptPath);
  if (runtime.empty()) {
    std::cerr << "No suitable runtime found for script: " << scriptPath
              << std::endl;
    return "";
  }
  return executeCGIScript(runtime);
}

std::string CGIHandler::executeCGIScript(const std::string& scriptPath) const {
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    std::cerr << "Failed to create pipe" << std::endl;
    return "";
  }

  pid_t pid = fork();
  if (pid == -1) {
    std::cerr << "Failed to fork process" << std::endl;
    close(pipefd[0]);
    close(pipefd[1]);
    return "";
  }

  if (pid == 0) {                                // Child process
    close(pipefd[0]);                            // Close unused read end
    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {  // Redirect stdout to pipe
      std::cerr << "Failed to redirect stdout to pipe" << std::endl;
      exit(EXIT_FAILURE);
    }
    close(pipefd[1]);  // Close write end of the pipe

    // Prepare environment
    char* envp[] = {NULL};

    // Prepare arguments
    std::vector<char*> argv;
    argv.push_back(const_cast<char*>(scriptPath.c_str()));
    argv.push_back(NULL);

    // Execute CGI script
    execve(scriptPath.c_str(), &argv[0], envp);
    std::cerr << "Failed to execute CGI script" << std::endl;
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
      std::cerr << "CGI script exited with error" << std::endl;
      return "";
    }
  }
}
