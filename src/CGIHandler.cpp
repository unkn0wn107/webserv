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

CGIHandler::CGIHandler(HTTPRequest& request, HTTPResponse& response): 
  _request(request),
  _response(response),
  _processOutput(""),
  _runtime(_identifyRuntime(_request)),
  _argv(CGIHandler::_getArgv(_request)),
  _envp(CGIHandler::_getEnvp(_request)),
  _pid(-2)
{
  _location = _request.getConfig();
  _root = _location->root;
  _index = _location->index;
  _cgi = _location->cgi;
  if (pipe(_inpipefd) == -1)
    throw PipeFailure("CGI: Failed to create pipe");
  if (pipe(_outpipefd) == -1)
    throw PipeFailure("CGI: Failed to create pipe");
}

CGIHandler::~CGIHandler() {
  Utils::freeCharVector(_argv);
  Utils::freeCharVector(_envp);
}

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

void CGIHandler::_checkIfProcessingPossible() {

    //  } catch (const Exception& e) {
    //   if (dynamic_cast<const CGIHandler::CGIDisabled*>(&e))
    //     response = new HTTPResponse(HTTPResponse::FORBIDDEN, &location);
    //   else if (dynamic_cast<const CGIHandler::TimeoutException*>(&e))
    //     response = new HTTPResponse(HTTPResponse::GATEWAY_TIMEOUT, &location);
    //   else if (dynamic_cast<const CGIHandler::ScriptNotFound*>(&e))
    //     response = new HTTPResponse(HTTPResponse::NOT_FOUND, &location);
    //   else
    //     response =
    //         new HTTPResponse(HTTPResponse::INTERNAL_SERVER_ERROR, &location);
    // }
  _log.info("CGI: checking if processing is possible for request: " + _request.getURI());
  _log.info("CGI: runtime: " + _runtime);
  _log.info("CGI: root: " + _root);
  _log.info("CGI: path: " + _request.getURIComponents().path);
  _log.info("CGI: index: " + _location->index);
  // _log.info("CGI: cgi: " + _location->cgi ? "true" : "false");
  if (!_cgi)
    throw CGIDisabled("Execution forbidden by config: " + _request.getURI());
  if (_runtime.empty())
    throw NoRuntimeError("No suitable runtime found for script: " +
                        _request.getURI());
  if (!FileManager::doesFileExists(_runtime))
    throw CGINotFound("CGI not found: " + _runtime);
  if (!FileManager::isFileExecutable(_runtime))
    throw CGINotExecutable("CGI not executable: " + _runtime);
  
  std::string scriptPath =
      _root + _request.getURIComponents().path;
  if (FileManager::isDirectory(scriptPath)) {
    if (scriptPath[scriptPath.length() - 1] != '/')
      scriptPath += "/";
    scriptPath += _location->index;
  }

  if (!FileManager::doesFileExists(scriptPath))
    throw ScriptNotFound("CGI: Script not found: " + scriptPath);
  if (!FileManager::isFileExecutable(scriptPath))
    throw ScriptNotExecutable("CGI: Script not executable: " + scriptPath);
}

int CGIHandler::handleCGIRequest() {
  int status = -1;
  try {
    _checkIfProcessingPossible();
  } catch (...) {
    throw;
  }
  std::string uriPath = _request.getURIComponents().path;
  _log.info("CGI: handling request for URI path: " + uriPath);

  bool noCache = (_request.getHeader("Cache-Control") == "no-cache");

  if (!noCache) {
    clock_t       cacheStart = clock();
    HTTPResponse* cachedResponse = _cacheHandler.getResponse(_request);
    clock_t       cacheEnd = clock();

    if (cachedResponse) {
      double cacheTimeTaken =
          double(cacheEnd - cacheStart) * 1000 / CLOCKS_PER_SEC;
      _log.info("CGI: Cache HIT [" + Utils::to_string(cacheTimeTaken) + " ms]");
      _response = *cachedResponse;
      delete cachedResponse;
      return SENDING;
    }
  } else {
    _log.info("CGI: no-cache required by client");
  }
  try {
    status = _processRequest();
  } catch (...) {
    throw;
  }
  if (noCache)
    _response.addHeader("Cache-Control", "no-cache");
  else {
    _response.addHeader(
        "Cache-Control",
        "public, max-age=" + Utils::to_string(CacheHandler::MAX_AGE));
    _cacheHandler.storeResponse(_request, _response);
  }
  return status;
}

int CGIHandler::_processRequest() {
  if (_pid == -2)
  {
    _log.info("CGI: Forking process");
    _pid = fork();
  }
  if (_pid == -1) {
    throw ForkFailure("CGI: Failed to fork process");
  } else if (_pid == 0) {
    try {
      _runScript();
    } catch (...) {
      throw;
    }
  } else {
    try {
      return _executeParentProcess();
    } catch (...) {
      throw;
    }
  }
  throw ExecutorError("CGI script fatal : out of pid branches !!");  // -Werror
}

const std::string CGIHandler::_identifyRuntime(const HTTPRequest& request) {
  std::string     extension = request.getURIComponents().extension;
  LocationConfig* location = request.getConfig();
  if (FileManager::isDirectory(location->root +
                               request.getURIComponents().path))
    extension = location->index.substr(location->index.find_last_of('.') + 1);

  for (int i = 0; i < CGIHandler::_NUM_AVAILABLE_CGIS; i++) {
    if (CGIHandler::_AVAILABLE_CGIS[i].first == extension) {
      return CGIHandler::_AVAILABLE_CGIS[i].second;
    }
  }
  return "";
}

int CGIHandler::_executeParentProcess() {
    close(_outpipefd[1]);
    char        buffer[1024];
    ssize_t     count;
    pid_t       pid;
    int         trys = 0;
    
    int status;
    pid = waitpid(_pid, &status, WNOHANG);
    if (pid == -1)
    {
      close(_outpipefd[0]);
      throw RuntimeError("CGI: waitpid failed");
    }
    // if (WIFSIGNALED(status)) {
    //   close(_outpipefd[0]);
    //   throw RuntimeError("CGI: script killed by signal: " + Utils::to_string(WTERMSIG(status)));
    // }
    if (pid == 0) {
      _log.info("CGI: script is still running");
      return EXECUTING;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      throw RuntimeError("CGI: script finished with errors, exit status: " + Utils::to_string(WEXITSTATUS(status)));
    }
    _log.info("CGI: script finished");
    while ((count = read(_outpipefd[0], buffer, sizeof(buffer))) > 0) {
      if (count == -1)
      {
          if (trys > 5) {
            close(_outpipefd[0]);
            throw ExecutorError("CGI: write failed: unable to write POST data to pipe after "
                                "multiple "
                                "retries");
          }
          trys++;
          usleep(1000 << trys);
          continue;
        }
      _processOutput.append(buffer, count);
    }
    close(_outpipefd[0]);
    std::size_t headerEndPos = _processOutput.find("\r\n\r\n");
    if (headerEndPos == std::string::npos)
      throw ExecutorError(
          "CGI: Invalid response: no header-body separator found");
    std::string headerPart = _processOutput.substr(0, headerEndPos);
    std::string bodyContent =
        _processOutput.substr(headerEndPos + 4);  // +4 to skip the "\r\n\r\n"

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
    _response.setHeaders(headers);
    _response.setBody(bodyContent);
    return SENDING;
}

void CGIHandler::_runScript() {
  _log.info("CGI: executing: " + _request.getURIComponents().scriptName +
            " with runtime: " + _runtime);
  close(_outpipefd[0]);
  if (dup2(_outpipefd[1], STDOUT_FILENO) == -1)
    throw ExecutorError("CGI: dup2 failed: unable to redirect stdout to pipe");
  close(_outpipefd[1]);
  if (dup2(_inpipefd[0], STDIN_FILENO) == -1)
    throw ExecutorError("CGI: dup2 failed: unable to redirect stdin to pipe");
  close(_inpipefd[0]);
  std::string postData = _request.getBody();
  std::size_t totalWritten = 0;
  int         trys = 0;  // Ajout d'un compteur de tentatives

  while (totalWritten < postData.size()) {
    ssize_t written = write(_inpipefd[1], postData.c_str() + totalWritten,
                            postData.size() - totalWritten);
    if (written == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        if (trys > 5) {
          close(_inpipefd[1]);
          throw ExecutorError(
              "CGI: write failed: unable to write POST data to pipe after "
              "multiple "
              "retries");
        }
        trys++;
        usleep(1000 << trys);  // Attendre un peu avant de réessayer
        continue;
      } else {
        close(_inpipefd[1]);
        throw ExecutorError(
            "CGI: write failed: unable to write POST data to pipe, error: " +
            std::string(strerror(errno)));
      }
    }
    trys = 0;
    totalWritten += written;
  }
  close(_inpipefd[1]);
  execve(_argv[0], &_argv[0], &_envp[0]);
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
