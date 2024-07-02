/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:05 by agaley            #+#    #+#             */
/*   Updated: 2024/06/28 13:43:38 by agaley           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

Logger&       CGIHandler::_log = Logger::getInstance();
CacheHandler& CGIHandler::_cacheHandler = CacheHandler::getInstance();

CGIHandler::CGIHandler(HTTPRequest& request, HTTPResponse& response, int epollSocket, LocationConfig& location, ConnectionHandler* connectionHandler):
  _epollSocket(epollSocket),
  _connectionHandler(connectionHandler),
  _request(request),
  _response(response),
  _location(location),
  _processOutput(""),
  _processOutputSize(0),
  _runtime(_identifyRuntime(_request, _location)),
  _done(false),
  _argv(CGIHandler::_getArgv(_request, _location)),
  _envp(CGIHandler::_getEnvp(_request, _location)),
  _pid(-2)
{
  _root = _location.root;
  _index = _location.index;
  _cgi = _location.cgi;
  if (pipe(_inpipefd) == -1)
    throw PipeFailure("CGI: Failed to create pipe");
  if (pipe(_outpipefd) == -1)
    throw PipeFailure("CGI: Failed to create pipe");
  if (pthread_mutex_init(&_mutex, NULL) != 0)
    throw MutexFailure("CGI: Failed to create mutex");

}

CGIHandler::~CGIHandler() {
  _log.info("CGI: Destroying handler");
  if (_inpipefd[0] != -1)
    close(_inpipefd[0]);
  if (_inpipefd[1] != -1)
    close(_inpipefd[1]);
  if (_outpipefd[0] != -1)
    close(_outpipefd[0]);
  if (_outpipefd[1] != -1)
    close(_outpipefd[1]);
  Utils::freeCharVector(_argv);
  Utils::freeCharVector(_envp);
  pthread_mutex_destroy(&_mutex);
  epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _outpipefd[0], NULL);
  close(_outpipefd[0]);
}

const std::pair<std::string, std::string> CGIHandler::_AVAILABLE_CGIS[] = {
    std::make_pair("php", "/usr/bin/php-cgi"),
    std::make_pair("py", "/usr/bin/python3"),
    std::make_pair("js", "/usr/bin/node"),
    std::make_pair("bla", "/usr/bin/ubuntu_cgi_tester")};

const int CGIHandler::_NUM_AVAILABLE_CGIS =
    sizeof(CGIHandler::_AVAILABLE_CGIS) /
    sizeof(std::pair<std::string, std::string>);

bool CGIHandler::isScript(const HTTPRequest& request, LocationConfig& location) {
  if (_identifyRuntime(request, location).empty())
    return false;
  return true;
}

void CGIHandler::_checkIfProcessingPossible() {
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
    scriptPath += _location.index;
  }

  if (!FileManager::doesFileExists(scriptPath))
    throw ScriptNotFound("CGI: Script not found: " + scriptPath);
  if (!FileManager::isFileExecutable(scriptPath))
    throw ScriptNotExecutable("CGI: Script not executable: " + scriptPath);
}

int CGIHandler::handleCGIRequest() {
  int status = -1;
  if (_pid == -2)
  {
    try {
        _checkIfProcessingPossible();
    } catch (...) {
      throw;
    }
  }
  std::string uriPath = _request.getURIComponents().path;
  _log.info("CGI: handling request for URI path: " + uriPath);

  std::string cacheControl = _request.getHeader("Cache-Control");
  _log.info("CGI: cacheControl: " + cacheControl);
  bool noCache = (cacheControl == "no-cache");
  if (_pid == -2 && !noCache) {
    _log.info("CGI: Checking cache");
    int cacheStatus = -1;
    try {
      cacheStatus = _cacheHandler.getResponse(_request, _response);
      _log.info("CGI: cacheStatus: " + Utils::to_string(cacheStatus));
    } catch (...) {
      throw;
    }
    if (cacheStatus == 1)
      return SENDING;
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
  else if (status == SENDING) {
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
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.ptr = _connectionHandler;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, _outpipefd[0], &event) == -1) {
      close(_outpipefd[0]);
      return CLOSED;
    }
    _log.info("CGI: add fd to epoll: " + Utils::to_string(event.data.fd));
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

const std::string CGIHandler::_identifyRuntime(const HTTPRequest& request, LocationConfig& location) {
  std::string     extension = request.getURIComponents().extension;
  if (FileManager::isDirectory(location.root +
                               request.getURIComponents().path))
    extension = location.index.substr(location.index.find_last_of('.') + 1);

  for (int i = 0; i < CGIHandler::_NUM_AVAILABLE_CGIS; i++) {
    if (CGIHandler::_AVAILABLE_CGIS[i].first == extension) {
      return CGIHandler::_AVAILABLE_CGIS[i].second;
    }
  }
  return "";
}

int CGIHandler::getCgifd() {
  return _outpipefd[0];
}

int CGIHandler::_executeParentProcess() {
  close(_outpipefd[1]);
  char        buffer[248];
  ssize_t     count;
  pid_t       pid = 0;
  bool        done = false;

  int status;
  pthread_mutex_lock(&_mutex);
  done = _done;
  pthread_mutex_unlock(&_mutex);
  if (!done && (pid = waitpid(_pid, &status, WNOHANG)) == -1)
  {
    _log.info("CGI: waitpid failed: " + std::string(strerror(errno)));
  }
  if (!done && pid == 0) {
    _log.info("CGI: script is still running");
  }
  if (pid > 0)
  {
    pthread_mutex_lock(&_mutex);
    _done = true;
    pthread_mutex_unlock(&_mutex);
  }
  if (done && WIFSIGNALED(status))
  {
    _log.info("CGI: script finished with signal: " + Utils::to_string(WTERMSIG(status)));
    _done = true;
    if (WEXITSTATUS(status) != 0)
      _log.info("CGI: script finished with errors, exit status: " + Utils::to_string(WEXITSTATUS(status)));
  }
  count = read(_outpipefd[0], buffer, sizeof(buffer));
  if (!done && count == -1)
  {
    _log.info("CGI: read failed: " + std::string(strerror(errno)));
    return EXECUTING;
  }
  if (count != -1)
  {
    _processOutputSize += count;
    _processOutput.append(buffer, count);
    _log.info("CGI: read " + Utils::to_string(count) + " bytes");
    _log.info("CGI: processOutputSize: " + Utils::to_string(_processOutputSize));
  }
  if (done)
  {
    _log.info("CGI: processOutput: " + _processOutput);
    _log.info("CGI: process done");
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
  return EXECUTING;
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

std::vector<char*> CGIHandler::_getArgv(const HTTPRequest& request, LocationConfig& location) {
  std::vector<char*> argv;

  std::string scriptPath =
      location.root + request.getURIComponents().path;
  if (FileManager::isDirectory(scriptPath)) {
    if (scriptPath[scriptPath.length() - 1] != '/')
      scriptPath += "/";
    scriptPath += location.index;
  }

  argv.reserve(3);
  argv.push_back(Utils::cstr(_identifyRuntime(request, location)));
  argv.push_back(Utils::cstr(scriptPath));
  argv.push_back(NULL);

  return argv;
}

std::vector<char*> CGIHandler::_getEnvp(const HTTPRequest& request, LocationConfig& location) {
  std::vector<char*> envp;

  const std::map<std::string, std::string> headers = request.getHeaders();
  const URI::Components uriComponents = request.getURIComponents();

  std::string scriptName = uriComponents.scriptName;
  std::string scriptPath = location.root + uriComponents.path;
  if (FileManager::isDirectory(scriptPath)) {
    if (scriptPath[scriptPath.length() - 1] != '/')
      scriptPath += "/";
    scriptPath += location.index;
    if (scriptName[scriptName.length() - 1] != '/')
      scriptName += "/";
    scriptName += location.index;
  }

  envp.reserve(headers.size() + 10 + 1);  // 10 env variables + NULL

  envp.push_back(Utils::cstr("REDIRECT_STATUS=200"));  // For php-cgi at least
  envp.push_back(Utils::cstr("DOCUMENT_ROOT=" + location.root));
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
