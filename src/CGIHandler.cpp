/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:05 by agaley            #+#    #+#             */
/*   Updated: 2024/06/27 15:32:34 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

Logger&       CGIHandler::_log = Logger::getInstance();
CacheHandler& CGIHandler::_cacheHandler = CacheHandler::getInstance();

CGIHandler::CGIHandler() {}

const std::pair<std::string, std::string> CGIHandler::_AVAILABLE_CGIS[] = {
    std::make_pair("php", "/usr/bin/php-cgi"),
    std::make_pair("py", "/usr/bin/python3"),
    std::make_pair("js", "/usr/bin/node"),
    std::make_pair("bla", "/usr/bin/ubuntu_cgi_tester")};

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

  std::string scriptPath =
      request.getConfig()->root + request.getURIComponents().path;
  if (FileManager::isDirectory(scriptPath)) {
    if (scriptPath[scriptPath.length() - 1] != '/')
      scriptPath += "/";
    scriptPath += request.getConfig()->index;
  }

  if (!FileManager::doesFileExists(scriptPath))
    throw ScriptNotFound("CGI: Script not found: " + scriptPath);
  if (!FileManager::isFileExecutable(scriptPath))
    throw ScriptNotExecutable("CGI: Script not executable: " + scriptPath);
}

HTTPResponse* CGIHandler::handleCGIRequest(HTTPRequest& request) {
  std::string uriPath = request.getURIComponents().path;
  _log.info("CGI: handling request for URI path: " + uriPath);

  bool noCache = (request.getHeader("Cache-Control") == "no-cache");

  if (!noCache) {
    clock_t       cacheStart = clock();
    HTTPResponse* cachedResponse = _cacheHandler.getResponse(request);
    clock_t       cacheEnd = clock();

    if (cachedResponse) {
      double cacheTimeTaken =
          double(cacheEnd - cacheStart) * 1000 / CLOCKS_PER_SEC;
      _log.info("CGI: Cache HIT [" + Utils::to_string(cacheTimeTaken) + " ms]");
      return cachedResponse;
    }
  } else {
    _log.info("CGI: no-cache required by client");
  }

  clock_t       processStart = clock();
  HTTPResponse* response = _processRequest(request);
  clock_t       processEnd = clock();
  double        processTimeTaken =
      double(processEnd - processStart) * 1000 / CLOCKS_PER_SEC;
  _log.info("CGI: Time to process request [" +
            Utils::to_string(processTimeTaken) + " ms]");

  if (noCache)
    response->addHeader("Cache-Control", "no-cache");
  else {
    response->addHeader(
        "Cache-Control",
        "public, max-age=" + Utils::to_string(CacheHandler::MAX_AGE));
    _cacheHandler.storeResponse(request, *response);
  }
  return response;
}

HTTPResponse* CGIHandler::_processRequest(const HTTPRequest& request) {
  std::string runtime = _identifyRuntime(request);
  _checkIfProcessingPossible(request, runtime);

  int pipefd[2];
  if (pipe(pipefd) == -1)
    throw PipeFailure("Failed to create pipe");

  _log.info("CGI: executing: " + request.getURIComponents().scriptName +
            " with runtime: " + runtime);

  std::vector<char*> argv = _getArgv(request);
  std::vector<char*> envp = _getEnvp(request);
  pid_t              pid = fork();
  if (pid == -1) {
    Utils::freeCharVector(argv);
    Utils::freeCharVector(envp);
    throw ForkFailure("CGI: Failed to fork process");
  } else if (pid == 0) {
    try {
      _executeChildProcess(request, pipefd, argv, envp);
      Utils::freeCharVector(argv);
      Utils::freeCharVector(envp);
    } catch (...) {
      Utils::freeCharVector(argv);
      Utils::freeCharVector(envp);
      throw;
    }
  } else {
    try {
      HTTPResponse* response = _executeParentProcess(pipefd, pid);
      Utils::freeCharVector(argv);
      Utils::freeCharVector(envp);
      return response;
    } catch (...) {
      Utils::freeCharVector(argv);
      Utils::freeCharVector(envp);
      throw;
    }
  }
  throw ExecutorError("CGI script fatal : out of pid branches !!");  // -Werror
}

const std::string CGIHandler::_identifyRuntime(const HTTPRequest& request) {
  LocationConfig* location = request.getConfig();
  std::string     extension = request.getURIComponents().extension;
  if (FileManager::isDirectory(request.getConfig()->root +
                               request.getURIComponents().path))
    extension = location->index.substr(location->index.find_last_of('.') + 1);

  for (int i = 0; i < CGIHandler::_NUM_AVAILABLE_CGIS; i++) {
    if (CGIHandler::_AVAILABLE_CGIS[i].first == extension) {
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
    throw ExecutorError("CGI: Select error");
  } else if (retval == 0) {
    kill(pid, SIGKILL);
    throw TimeoutException("CGI: script execution timed out");
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
      throw RuntimeError("CGI: script failed");
    }
    // Parse output to create HTTPResponse
    std::size_t headerEndPos = output.find("\r\n\r\n");
    if (headerEndPos == std::string::npos)
      throw ExecutorError(
          "CGI: Invalid response: no header-body separator found");
    std::string headerPart = output.substr(0, headerEndPos);
    std::string bodyContent =
        output.substr(headerEndPos + 4);  // +4 to skip the "\r\n\r\n"

    std::map<std::string, std::string> headers;
    std::istringstream                 headerStream(headerPart);
    std::string                        line;
    while (std::getline(headerStream, line)) {
      std::size_t colonPos = line.find(':');
      if (colonPos == std::string::npos)
        throw ExecutorError("CGI: Invalid response: malformed header line");
      std::string key = line.substr(0, colonPos);
      if (key.empty())
        throw ExecutorError("CGI: Invalid response: empty header key");
      std::string value = line.substr(colonPos + 2);  // +2 to skip ": "
      if (value.empty())
        throw ExecutorError("CGI: Invalid response: empty header value");
      headers[key] = value;
    }
    headers["Content-Length"] = Utils::to_string(bodyContent.size());
    HTTPResponse* response = new HTTPResponse(HTTPResponse::OK);
    response->setHeaders(headers);
    response->setBody(bodyContent);
    return response;
  }
  throw ExecutorError("CGI: script failed : out of timeout loop");
}

void CGIHandler::_executeChildProcess(const HTTPRequest& request,
                                      int                pipefd[2],
                                      std::vector<char*> argv,
                                      std::vector<char*> envp) {
  close(pipefd[0]);  // Close the read end of the pipe
  // Redirect stdout to pipe
  if (dup2(pipefd[1], STDOUT_FILENO) == -1)
    throw ExecutorError("CGI: dup2 failed: unable to redirect stdout to pipe");
  close(pipefd[1]);

  // Handling POST data by redirecting stdin to a pipe
  int postPipe[2];
  if (pipe(postPipe) == -1)
    throw PipeFailure("CGI: pipe failed: unable to create pipe for POST data");
  if (dup2(postPipe[0], STDIN_FILENO) == -1)
    throw ExecutorError("CGI: dup2 failed: unable to redirect stdin to pipe");
  close(postPipe[0]);  // Close the read end of the pipe

  // Write POST data to postPipe[1] so it can be read from postPipe[0]
  std::string postData = request.getBody();
  std::size_t totalWritten = 0;
  int         trys = 0;  // Ajout d'un compteur de tentatives

  while (totalWritten < postData.size()) {
    ssize_t written = write(postPipe[1], postData.c_str() + totalWritten,
                            postData.size() - totalWritten);
    if (written == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        if (trys > 3) {
          close(postPipe[1]);
          throw ExecutorError(
              "CGI: write failed: unable to write POST data to pipe after "
              "multiple "
              "retries");
        }
        trys++;
        usleep(1000);  // Attendre un peu avant de réessayer
        continue;
      } else {
        close(postPipe[1]);
        throw ExecutorError(
            "CGI: write failed: unable to write POST data to pipe, error: " +
            std::string(strerror(errno)));
      }
    }
    trys = 0;  // Réinitialiser le compteur de tentatives après une écriture
               // réussie
    totalWritten += written;
  }
  close(postPipe[1]);
  execve(argv[0], &argv[0], &envp[0]);
  throw ExecutorError("CGI: execve failed: unable to execute CGI script");
}

std::vector<char*> CGIHandler::_getArgv(const HTTPRequest& request) {
  std::vector<char*> argv;

  std::string scriptPath =
      request.getConfig()->root + request.getURIComponents().path;
  if (FileManager::isDirectory(scriptPath)) {
    if (scriptPath[scriptPath.length() - 1] != '/')
      scriptPath += "/";
    scriptPath += request.getConfig()->index;
  }

  argv.reserve(3);
  argv.push_back(Utils::cstr(_identifyRuntime(request)));
  argv.push_back(Utils::cstr(scriptPath));
  argv.push_back(NULL);

  return argv;
}

std::vector<char*> CGIHandler::_getEnvp(const HTTPRequest& request) {
  std::vector<char*> envp;

  const std::map<std::string, std::string> headers = request.getHeaders();
  const URI::Components uriComponents = request.getURIComponents();

  std::string scriptName = uriComponents.scriptName;
  std::string scriptPath = request.getConfig()->root + uriComponents.path;
  if (FileManager::isDirectory(scriptPath)) {
    if (scriptPath[scriptPath.length() - 1] != '/')
      scriptPath += "/";
    scriptPath += request.getConfig()->index;
    if (scriptName[scriptName.length() - 1] != '/')
      scriptName += "/";
    scriptName += request.getConfig()->index;
  }

  envp.reserve(headers.size() + 10 + 1);  // 10 env variables + NULL

  envp.push_back(Utils::cstr("REDIRECT_STATUS=200"));  // For php-cgi at least
  envp.push_back(Utils::cstr("DOCUMENT_ROOT=" + request.getConfig()->root));
  envp.push_back(Utils::cstr("REQUEST_URI=" + request.getURI()));
  envp.push_back(Utils::cstr("SCRIPT_FILENAME=" + scriptPath));
  envp.push_back(Utils::cstr("SCRIPT_NAME=" + scriptName));
  envp.push_back(Utils::cstr("PATH_INFO=" + uriComponents.pathInfo));
  envp.push_back(Utils::cstr("REQUEST_METHOD=" + request.getMethod()));
  envp.push_back(Utils::cstr("QUERY_STRING=" + uriComponents.query));
  envp.push_back(Utils::cstr("REMOTE_HOST=localhost"));
  envp.push_back(Utils::cstr("CONTENT_LENGTH=" +
                             Utils::to_string(request.getBody().length())));

  for (std::map<std::string, std::string>::const_iterator hd = headers.begin();
       hd != headers.end(); ++hd) {
    std::string envName = "HTTP_" + hd->first;
    std::replace(envName.begin(), envName.end(), '-', '_');
    std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
    std::string envLine = envName + "=" + hd->second;
    envp.push_back(Utils::cstr(envLine));
  }
  envp.push_back(NULL);

  return envp;
}
