/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:05 by agaley            #+#    #+#             */
/*   Updated: 2024/07/23 01:08:08 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

Logger& CGIHandler::_log = Logger::getInstance();

CGIHandler::CGIHandler(HTTPRequest&          request,
                       HTTPResponse&         response,
                       int                   epollSocket,
                       const LocationConfig& location)
    : _cacheHandler(CacheHandler::getInstance()),
      _state(REGISTER_SCRIPT_FD),
      _epollSocket(epollSocket),
      _eventData(NULL),
      _request(request),
      _response(response),
      _location(location),
      _runStartTime(std::time(NULL)),
      _processOutput(""),
      _processOutputSize(0),
      _runtime(_identifyRuntime(_request, _location)),
      _argv(_buildScriptArguments(_request, _location)),
      _envp(_buildScriptEnvironment(_request, _location)),
      _pid(-2) {
  _root = _location.root;
  _index = _location.index;
  _cgi = _location.cgi;
  _outpipefd[0] = -1;
  _outpipefd[1] = -1;
  _inpipefd[0] = -1;
  _inpipefd[1] = -1;
}

CGIHandler::~CGIHandler() {
  kill(_pid, SIGKILL);
  waitpid(_pid, NULL, 0);
  if (_inpipefd[0] != -1)
  {
    delEvent();
    close(_inpipefd[0]);
  }
  if (_inpipefd[1] != -1)
    close(_inpipefd[1]);
  if (_outpipefd[0] != -1)
    delEvent();
  if (_outpipefd[1] != -1)
    close(_outpipefd[1]);
}

const std::pair<std::string, std::string> CGIHandler::_AVAILABLE_CGIS[] = {
    std::make_pair("php", "/usr/bin/php-cgi"), std::make_pair("py", "/usr/bin/python3"),
    std::make_pair("js", "/usr/bin/node"),
    std::make_pair("bla", "/usr/bin/ubuntu_cgi_tester")};

const int CGIHandler::_NUM_AVAILABLE_CGIS =
    sizeof(CGIHandler::_AVAILABLE_CGIS) / sizeof(std::pair<std::string, std::string>);

bool CGIHandler::isScript(const HTTPRequest& request, const LocationConfig& location) {
  if (_identifyRuntime(request, location).empty())
    return false;
  return true;
}

int CGIHandler::getCgifd() {
  return _outpipefd[0];
}

CGIState CGIHandler::getCgiState() {
  return _state;
}

void CGIHandler::_checkIfProcessingPossible() {
  if (!_cgi)
    throw CGIDisabled("Execution forbidden by config: " + _request.getURI());
  if (_runtime.empty())
    throw NoRuntimeError("No suitable runtime found for script: " + _request.getURI());
  if (!FileManager::doesFileExists(_runtime))
    throw CGINotFound("CGI not found: " + _runtime);
  if (!FileManager::isFileExecutable(_runtime))
    throw CGINotExecutable("CGI not executable: " + _runtime);

  std::string scriptPath = _root + _request.getURIComponents().path;
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

void CGIHandler::_runScript() {
  if (dup2(_outpipefd[1], STDOUT_FILENO) == -1) {
    std::cerr << "CHILD: dup2 failed: unable to redirect stdout to pipe" << std::endl;
    exit(EXIT_FAILURE);
  }
  close(_outpipefd[0]);
  close(_outpipefd[1]);
  std::string postData = _request.getBody();
  if (!postData.empty()) {
    if (pipe(_inpipefd) == -1) {
      std::cerr << "CHILD: Failed to create pipe" << std::endl;
      exit(EXIT_FAILURE);
    }
    if (dup2(_inpipefd[0], STDIN_FILENO) == -1) {
      std::cerr << "CHILD: dup2 failed: unable to redirect stdin to pipe" << std::endl;
      exit(EXIT_FAILURE);
    }
    close(_inpipefd[0]);
    std::size_t totalWritten = 0;
    int         trys = 0;

    while (totalWritten < postData.size()) {
      ssize_t written = write(_inpipefd[1], postData.c_str() + totalWritten,
                              postData.size() - totalWritten);

      if (written == -1) {
        if (trys > 5) {
          close(_inpipefd[1]);
          std::cerr << "CHILD: write failed: unable to write POST data to pipe "
                       "after "
                       "multiple "
                       "retries"
                    << std::endl;
          exit(EXIT_FAILURE);
        }
        trys++;
        usleep(1000 << trys);  // Attendre un peu avant de réessayer
        continue;
      }
      trys = 0;
      totalWritten += written;
    }
    close(_inpipefd[1]);
  }

  char** argv_ptrs = new char*[_argv.size() + 1];
  char** envp_ptrs = new char*[_envp.size() + 1];

  for (size_t i = 0; i < _argv.size(); ++i)
    argv_ptrs[i] = const_cast<char*>(_argv[i].data());
  argv_ptrs[_argv.size()] = NULL;

  std::string execDir;
  for (size_t i = 0; i < _envp.size(); ++i) {
    if (_envp[i].find("DOCUMENT_ROOT=") != std::string::npos)
      execDir = _envp[i].substr(14);
    envp_ptrs[i] = const_cast<char*>(_envp[i].data());
  }
  envp_ptrs[_envp.size()] = NULL;

  if (chdir(execDir.data()) == -1) {
    std::cerr << "CHILD: chdir failed: unable to execute CGI script" << std::endl;
    delete[] argv_ptrs;
    delete[] envp_ptrs;
    exit(EXIT_FAILURE);
  }
  if (execve(argv_ptrs[0], argv_ptrs, envp_ptrs) == -1) {
    std::cerr << "CHILD: execve failed: unable to execute CGI script" << std::endl;
    delete[] argv_ptrs;
    delete[] envp_ptrs;
    exit(EXIT_FAILURE);
  }
}

ConnectionStatus CGIHandler::handleCGIRequest() {
  if (_state == REGISTER_SCRIPT_FD) {
    try {
      _checkIfProcessingPossible();
      if (pipe(_outpipefd) == -1)
        throw PipeFailure("CGI: Failed to create pipe");
      _pid = fork();
      if (_pid == -1)
        throw ForkFailure("CGI: Failed to fork process");
      if (_pid > 0) {
        close(_outpipefd[1]);
        _outpipefd[1] = -1;
        _runStartTime = std::time(NULL);
        _state = SCRIPT_RUNNING;
        _createEvent();
        return EXECUTING;
      } else if (_pid == 0)
        _state = RUN_SCRIPT;
    } catch (const Exception& e) {
      if (dynamic_cast<const CGIHandler::CGIDisabled*>(&e) ||
          dynamic_cast<const CGIHandler::CGINotExecutable*>(&e) ||
          dynamic_cast<const CGIHandler::ScriptNotExecutable*>(&e))
        _response.setStatusCode(HTTPResponse::FORBIDDEN);
      else if (dynamic_cast<const CGIHandler::NoRuntimeError*>(&e) ||
               dynamic_cast<const CGIHandler::CGINotFound*>(&e) ||
               dynamic_cast<const CGIHandler::ScriptNotFound*>(&e))
        _response.setStatusCode(HTTPResponse::NOT_FOUND);
      _state = CGI_ERROR;
    }
  }
  if (_state == RUN_SCRIPT) {
    try {
      _runScript();
    } catch (...) {
      exit(EXIT_FAILURE);
    }
  }
  if (_state == SCRIPT_RUNNING) {
    try {
      pid_t pidReturn = 0;
      int   status = -1;

      if ((pidReturn = waitpid(_pid, &status, WNOHANG)) == -1)
        throw ExecutorError("CGI: waitpid failed: " + std::string(strerror(errno)));
      if (pidReturn == 0) {
        usleep(1000);
        return EXECUTING;
      }
      if (WIFEXITED(status)) {
        int exitStatus = WEXITSTATUS(status);
        if (exitStatus != 0)
          throw ExecutorError("CGI: script finished with errors, exit status: " +
                              Utils::to_string(exitStatus));
        _state = READ_FROM_CGI;
      }
      return EXECUTING;
    } catch (const Exception& e) {
      _state = CGI_ERROR;
    }
  }
  if (_state == READ_FROM_CGI) {
    try {
      char    buffer[BUFFER_SIZE];
      ssize_t count = 0;
      count = read(_outpipefd[0], buffer, sizeof(buffer));
      if (count == -1) {
        _log.warning("CGI: read failed");
        return EXECUTING;
      }
      _processOutput.append(buffer, count);
      _processOutputSize += count;
      if (count < (ssize_t)sizeof(buffer) || count == 0) {
        delEvent();
        _state = PROCESS_OUTPUT;
      } else
        return EXECUTING;
    } catch (...) {
      _state = CGI_ERROR;
    }
  }
  if (_state == PROCESS_OUTPUT) {
    try {
      std::string            normalizedOutput = _processOutput;
      std::string::size_type pos = 0;
      while ((pos = normalizedOutput.find('\n', pos)) != std::string::npos) {
        if (pos == 0 || normalizedOutput[pos - 1] != '\r') {
          normalizedOutput.replace(pos, 1, "\r\n");
          pos += 2;  // Skip past the new \r\n
        } else {
          ++pos;  // Already \r\n, move to the next character
        }
      }
      std::size_t headerEndPos = normalizedOutput.find("\r\n\r\n");
      if (headerEndPos == std::string::npos)
        throw ExecutorError("CGI: Invalid response: no header-body separator found");
      std::string headerPart = normalizedOutput.substr(0, headerEndPos);
      std::string bodyContent =
          normalizedOutput.substr(headerEndPos + 4);  // +4 to skip the "\r\n\r\n"

      std::map<std::string, std::string> headers = _parseOutputHeaders(headerPart);
      headers["Content-Length"] = Utils::to_string(bodyContent.size());
      _response.setHeaders(headers);
      _response.addHeader("Cache-Control",
                          "public, max-age=" + Utils::to_string(CacheHandler::MAX_AGE));
      _response.setBody(bodyContent);
      _cacheHandler.storeResponse(_cacheHandler.generateKey(_request), _response);
      return SENDING;
    } catch (...) {
      _state = CGI_ERROR;
    }
  }

  if (_state == CGI_ERROR) {
    if (_response.getStatusCode() == HTTPResponse::OK)
      _response.setStatusCode(HTTPResponse::INTERNAL_SERVER_ERROR);
    return SENDING;
  }
  return EXECUTING;
}

void CGIHandler::_createEvent() {
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
  event.data.ptr = _eventData;
  if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, _outpipefd[0], &event) == -1) {
    _log.error(std::string("CGI_HANDLER: create epoll_ctl: ") + strerror(errno));
  }
}

void CGIHandler::delEvent() {
  if (epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _outpipefd[0], NULL) == -1) {
    _log.error(std::string("CGI_HANDLER: delete epoll_ctl: ") + strerror(errno));
  }
  close(_outpipefd[0]);
  _outpipefd[0] = -1;
}

std::map<std::string, std::string> CGIHandler::_parseOutputHeaders(
    const std::string& headerPart) {
  std::map<std::string, std::string> headers;
  std::istringstream                 headerStream(headerPart);
  std::string                        line;
  while (std::getline(headerStream, line)) {
    std::size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
      throw ExecutorError("CGI: Invalid response: malformed header line");
    }
    std::string key = line.substr(0, colonPos);
    if (key.empty()) {
      throw ExecutorError("CGI: Invalid response: empty header key");
    }
    std::string value = line.substr(colonPos + 2);  // +2 to skip ": "
    if (value.empty()) {
      throw ExecutorError("CGI: Invalid response: empty header value");
    }
    headers[key] = value;
  }
  return headers;
}

std::vector<std::string> CGIHandler::_buildScriptArguments(
    const HTTPRequest&    request,
    const LocationConfig& location) {
  std::vector<std::string> argv;

  std::string scriptPath = location.root + request.getURIComponents().path;
  if (FileManager::isDirectory(scriptPath)) {
    if (scriptPath[scriptPath.length() - 1] != '/')
      scriptPath += "/";
    scriptPath += location.index;
  }

  argv.reserve(2);
  argv.push_back(_identifyRuntime(request, location));
  argv.push_back(scriptPath);

  return argv;
}

std::vector<std::string> CGIHandler::_buildScriptEnvironment(
    const HTTPRequest&    request,
    const LocationConfig& location) {
  std::vector<std::string> envp;

  const std::map<std::string, std::string>& headers = request.getHeaders();
  const URI::Components&                    uriComponents = request.getURIComponents();

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

  envp.reserve(headers.size() + 10);  // 10 env variables

  envp.push_back("REDIRECT_STATUS=200");  // For php-cgi at least
  envp.push_back("DOCUMENT_ROOT=" + location.root);
  envp.push_back("REQUEST_URI=" + request.getURI());
  envp.push_back("SCRIPT_FILENAME=" + scriptPath);
  envp.push_back("SCRIPT_NAME=" + scriptName);
  envp.push_back("PATH_INFO=" + uriComponents.pathInfo);
  envp.push_back("REQUEST_METHOD=" + request.getMethod());
  envp.push_back("QUERY_STRING=" + uriComponents.query);
  envp.push_back("REMOTE_HOST=localhost");
  envp.push_back("CONTENT_LENGTH=" + Utils::to_string(request.getBody().length()));

  for (std::map<std::string, std::string>::const_iterator hd = headers.begin();
       hd != headers.end(); ++hd) {
    std::string envName = "HTTP_" + hd->first;
    std::replace(envName.begin(), envName.end(), '-', '_');
    std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
    envp.push_back(envName + "=" + hd->second);
  }

  return envp;
}

const std::string CGIHandler::_identifyRuntime(const HTTPRequest&    request,
                                               const LocationConfig& location) {
  std::string extension = request.getURIComponents().extension;
  if (FileManager::isDirectory(location.root + request.getURIComponents().path))
    extension = location.index.substr(location.index.find_last_of('.') + 1);

  for (int i = 0; i < CGIHandler::_NUM_AVAILABLE_CGIS; i++) {
    if (CGIHandler::_AVAILABLE_CGIS[i].first == extension) {
      return CGIHandler::_AVAILABLE_CGIS[i].second;
    }
  }
  return "";
}
