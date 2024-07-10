/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:05 by agaley            #+#    #+#             */
/*   Updated: 2024/07/04 22:22:10 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "CGIHandler.hpp"

Logger&       CGIHandler::_log = Logger::getInstance();
CacheHandler&  CGIHandler::_cacheHandler = CacheHandler::getInstance();

CGIHandler::CGIHandler(HTTPRequest&          request,
                       HTTPResponse&         response,
                       int                   epollSocket,
                       const LocationConfig& location,
                       ConnectionHandler*    connectionHandler)
    : _state(INIT),
      _epollSocket(epollSocket),
      _connectionHandler(connectionHandler),
      _eventData(NULL),
      _request(request),
      _response(response),
      _location(location),
      _runStartTime(std::time(NULL)),
      _processOutput(""),
      _processOutputSize(0),
      _runtime(_identifyRuntime(_request, _location)),
      _done(false),
      _argv(CGIHandler::_buildScriptArguments(_request, _location)),
      _envp(CGIHandler::_buildScriptEnvironment(_request, _location)),
      _pid(-2) {
  _root = _location.root;
  _index = _location.index;
  _cgi = _location.cgi;
  if (pipe(_inpipefd) == -1)
    throw PipeFailure("CGI: Failed to create pipe");
  if (pipe(_outpipefd) == -1)
    throw PipeFailure("CGI: Failed to create pipe");
}

CGIHandler::~CGIHandler() {
  _log.info("CGI: Destroying handler");
  epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _outpipefd[0], NULL);
  if (_eventData)
    delete _eventData;
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
}

const std::pair<std::string, std::string> CGIHandler::_AVAILABLE_CGIS[] = {
    std::make_pair("php", "/usr/bin/php-cgi"),
    std::make_pair("py", "/usr/bin/python3"),
    std::make_pair("js", "/usr/bin/node"),
    std::make_pair("bla", "/usr/bin/ubuntu_cgi_tester")};

const int CGIHandler::_NUM_AVAILABLE_CGIS =
    sizeof(CGIHandler::_AVAILABLE_CGIS) /
    sizeof(std::pair<std::string, std::string>);

bool CGIHandler::isScript(const HTTPRequest&    request,
                          const LocationConfig& location) {
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
    throw NoRuntimeError("No suitable runtime found for script: " +
                         _request.getURI());
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
    ssize_t written =
        write(_inpipefd[1], postData.c_str() + totalWritten,
              postData.size() - totalWritten);
    if (written == -1) {
        if (trys > 5) {
          close(_inpipefd[1]);
          throw ExecutorError(
              "CGI: write failed: unable to write POST data to pipe "
              "after "
              "multiple "
              "retries");
        }
        trys++;
        usleep(1000 << trys);  // Attendre un peu avant de réessayer
        continue;
    }
    trys = 0;
    totalWritten += written;
  }
  close(_inpipefd[1]);
  execve(_argv[0], &_argv[0], &_envp[0]);
  _log.error("CGI: execve failed: unable to execute CGI script");
  exit(EXIT_FAILURE);
}

ConnectionStatus CGIHandler::handleCGIRequest() {
  if (_state == INIT){
    // std::string cacheControl;
    try {
      // cacheControl = _request.getHeader("Cache-Control");
      // _log.info("CGI: cacheControl: " +
      //           (cacheControl.empty() ? "Empty header = cache active" : cacheControl));
      // if (cacheControl.empty())
      //   _state = CACHE_CHECK;
      // else
        _state = REGISTER_SCRIPT_FD;
    } catch (const Exception& e) {
      if (dynamic_cast<const CGIHandler::CGIDisabled*>(&e) || dynamic_cast<const CGIHandler::CGINotExecutable*>(&e) || dynamic_cast<const CGIHandler::ScriptNotExecutable*>(&e))
        _response.setStatusCode(HTTPResponse::FORBIDDEN);
      else if (dynamic_cast<const CGIHandler::NoRuntimeError*>(&e) || dynamic_cast<const CGIHandler::CGINotFound*>(&e) || dynamic_cast<const CGIHandler::ScriptNotFound*>(&e))
          _response.setStatusCode(HTTPResponse::NOT_FOUND);
      _state = CGI_ERROR;
    }
  }
  // if (_state == CACHE_CHECK){
  //   std::string uriPath;
  //   int cacheStatus;
  //   try {
  //     uriPath = _request.getURIComponents().path;
  //     cacheStatus = _cacheHandler.getResponse(_request, _response);
  //     _log.info("CGI: For URI path: " + uriPath + ", cacheStatus: " +
  //               (cacheStatus == CACHE_CURRENTLY_BUILDING  ? "Cache currently building"
  //               : cacheStatus == CACHE_FOUND ? "Cache found"
  //                                   : "Cache not found"));
  //     if (cacheStatus == CACHE_FOUND)
  //       return SENDING;
  //     if (cacheStatus == CACHE_CURRENTLY_BUILDING)
  //       return EXECUTING;
  //     if (cacheStatus == CACHE_NOT_FOUND) {
  //       _cacheHandler.reserveCache(_request); // Cache-building waiting, first caller registers NULL
  //       _state = REGISTER_SCRIPT_FD;
  //     }
  //   } catch (...) {
  //     _state = CGI_ERROR;
  //   }
  // }
  if (_state == REGISTER_SCRIPT_FD){  // Set-up execution environment
    try {
      _checkIfProcessingPossible();
      _pid = fork();
      if (_pid == -1)
        throw ForkFailure("CGI: Failed to fork process");
      if (_pid > 0) {
        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        _eventData = new EventData(_outpipefd[0], _connectionHandler);
        event.data.ptr = _eventData;
        event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, _outpipefd[0], &event) == -1) {
          close(_outpipefd[0]);
          delete _eventData;
          _state = CGI_ERROR;
          
        } else {
          close(_inpipefd[0]);
          close(_outpipefd[1]);
          _state = SCRIPT_RUNNING;
          _runStartTime = std::time(NULL);
        }
      } else if (_pid == 0)
        _state = RUN_SCRIPT;
    } catch (...) {
      _state = CGI_ERROR;
    }
  }
  if (_state == RUN_SCRIPT){
    try {
      _runScript();
    } catch (...) {
      exit(EXIT_FAILURE);
    }
  }
  if (_state == SCRIPT_RUNNING){
    try {
      pid_t   pidReturn = 0;
      int     status = -1;

      if ((pidReturn = waitpid(_pid, &status, WNOHANG)) == -1)
        throw ExecutorError("CGI: waitpid failed: " + std::string(strerror(errno)));
      if (pidReturn == 0) {
        _log.info("CGI: script is still running");
        if (std::time(NULL) - _runStartTime > CGI_TIMEOUT_SEC) {
          kill(_pid, SIGKILL);
          int statusClean;
          waitpid(_pid, &statusClean, 0); // To stop child process in Zombie state
          throw TimeoutException("CGI: script timedout after " +
                                Utils::to_string(std::time(NULL) - _runStartTime) + " seconds");
        }
        return EXECUTING;
      }
      if (WIFEXITED(status)) {
        int exitStatus = WEXITSTATUS(status);
        _log.info("CGI: script exited, status=" + Utils::to_string(exitStatus));
        if (exitStatus != 0)
          throw ExecutorError("CGI: script finished with errors, exit status: " + Utils::to_string(exitStatus));
        _state = READ_FROM_CGI;
      }
      return EXECUTING;
    } catch (const Exception& e) {
      if (dynamic_cast<const CGIHandler::TimeoutException*>(&e))
        _response.setStatusCode(HTTPResponse::GATEWAY_TIMEOUT);
      _state = CGI_ERROR;
    }
  }
  if (_state == READ_FROM_CGI){
    try {
      close(_outpipefd[1]);
      char    buffer[512];
      ssize_t count = 0;
      count = read(_outpipefd[0], buffer, sizeof(buffer));
      if (count == -1) {
        _log.warning("CGI: read failed: " + std::string(strerror(errno)));
        return EXECUTING;
      } 
      _processOutput.append(buffer, count);
      _processOutputSize += count;
      if (count < (ssize_t)sizeof(buffer) || count == 0)
        _state = PROCESS_OUTPUT;
      else
        return EXECUTING;
    } catch (...) {
      _state = CGI_ERROR;
    }
  }
  if (_state == PROCESS_OUTPUT){
    try {
      std::string normalizedOutput = _processOutput;
      std::string::size_type pos = 0;
      while ((pos = normalizedOutput.find('\n', pos)) != std::string::npos) {
        if (pos == 0 || normalizedOutput[pos - 1] != '\r') {
            normalizedOutput.replace(pos, 1, "\r\n");
            pos += 2; // Skip past the new \r\n
        } else {
            ++pos; // Already \r\n, move to the next character
        }
      }
      _log.info("CGI: processOutput: " + normalizedOutput);
      std::size_t headerEndPos = normalizedOutput.find("\r\n\r\n");
      if (headerEndPos == std::string::npos)
        throw ExecutorError(
            "CGI: Invalid response: no header-body separator found");
      std::string headerPart = normalizedOutput.substr(0, headerEndPos);
      std::string bodyContent = normalizedOutput.substr(
          headerEndPos + 4);  // +4 to skip the "\r\n\r\n"

      std::map<std::string, std::string> headers =
          _parseOutputHeaders(headerPart);
      headers["Content-Length"] = Utils::to_string(bodyContent.size());
      _response.setHeaders(headers);
      _response.setBody(bodyContent);
      _state = FINALIZE_RESPONSE;
    } catch (...) {
      _state = CGI_ERROR;
    }
  }
  if (_state == FINALIZE_RESPONSE){
    try {
      if (_request.getHeader("Cache-Control") == "no-cache")
        _response.addHeader("Cache-Control", "no-cache");
      else {
        _response.addHeader(
            "Cache-Control",
            "public, max-age=" + Utils::to_string(CacheHandler::MAX_AGE));
      }
      return SENDING;
    } catch (...) {
      _state = CGI_ERROR;
    }
  }
  if (_state == CGI_ERROR){
    if (_response.getStatusCode() == HTTPResponse::OK)
          _response.setStatusCode(HTTPResponse::INTERNAL_SERVER_ERROR);
    _cacheHandler.deleteCache(_request);
    return SENDING;
  }
  return EXECUTING;
}

std::map<std::string, std::string> CGIHandler::_parseOutputHeaders(
    const std::string& headerPart) {
  std::map<std::string, std::string> headers;
  std::istringstream                 headerStream(headerPart);
  std::string                        line;
  while (std::getline(headerStream, line)) {
    std::size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
      _log.error("CGI: Malformed header line: " + line); // Log the problematic line
      throw ExecutorError("CGI: Invalid response: malformed header line");
    }
    std::string key = line.substr(0, colonPos);
    if (key.empty()) {
      _log.error("CGI: Empty header key in line: " + line); // Log the problematic line
      throw ExecutorError("CGI: Invalid response: empty header key");
    }
    std::string value = line.substr(colonPos + 2);  // +2 to skip ": "
    if (value.empty()) {
      _log.error("CGI: Empty header value in line: " + line); // Log the problematic line
      throw ExecutorError("CGI: Invalid response: empty header value");
    }
    headers[key] = value;
  }
  return headers;
}

std::vector<char*> CGIHandler::_buildScriptArguments(
    const HTTPRequest&    request,
    const LocationConfig& location) {
  std::vector<char*> argv;

  std::string scriptPath = location.root + request.getURIComponents().path;
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

std::vector<char*> CGIHandler::_buildScriptEnvironment(
    const HTTPRequest&    request,
    const LocationConfig& location) {
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
