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
#include "EventData.hpp"

Logger&       CGIHandler::_log = Logger::getInstance();
CacheHandler& CGIHandler::_cacheHandler = CacheHandler::getInstance();

CGIHandler::CGIHandler(HTTPRequest&          request,
                       HTTPResponse&         response,
                       int                   epollSocket,
                       const LocationConfig& location,
                       ConnectionHandler*    connectionHandler)
    : _state(INIT),
      _epollSocket(epollSocket),
      _connectionHandler(connectionHandler),
      _request(request),
      _response(response),
      _location(location),
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
  if (pthread_mutex_init(&_mutex, NULL) != 0)
    throw MutexFailure("CGI: Failed to create mutex");
}

CGIHandler::~CGIHandler() {
  _log.info("CGI: Destroying handler");
  epoll_ctl(_epollSocket, EPOLL_CTL_DEL, _outpipefd[0], NULL);
  close(_outpipefd[0]);
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

ConnectionStatus CGIHandler::mapStateToConnectionStatus() const {
  switch (_state) {
    case DONE:
      return SENDING;
    case CGI_ERROR:
      return ERROR;
    default:
      return EXECUTING;
  }
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

ConnectionStatus CGIHandler::handleCGIRequest() {
  switch (_state) {
    case INIT:
      try {
        _checkIfProcessingPossible();
        std::string cacheControl = _request.getHeader("Cache-Control");
        _log.info("CGI: cacheControl: " +
                  (cacheControl.empty() ? "Empty header" : cacheControl));

        if (cacheControl.empty())
          _state = CACHE_CHECK;
        else
          _state = PROCESS_SETUP;
        return handleCGIRequest();
      } catch (...) {
        throw;
      }
      break;

    case CACHE_CHECK:  // 3 types : no cache, cache currently building and not
                       // found
      try {
        std::string uriPath = _request.getURIComponents().path;
        int cacheStatus = _cacheHandler.getResponse(_request, _response);
        _log.info("CGI: For URI path: " + uriPath + ", cacheStatus: " +
                  (cacheStatus == -1  ? "Cache currently building"
                   : cacheStatus == 1 ? "Cache found"
                                      : "Cache not found"));
        if (cacheStatus == 1)
          _state = DONE;
        // else if (cacheStatus == -1)
        //   _state = CACHE_CHECK;
        else
          _state = PROCESS_SETUP;
        return handleCGIRequest();
      } catch (...) {
        throw;
      }
      break;

    case PROCESS_SETUP:  // Set-up execution environment
      try {
        struct epoll_event event;
        memset(&event, 0, sizeof(event));
        event.data.ptr = new EventData(_outpipefd[0], _connectionHandler);
        event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        if (epoll_ctl(_epollSocket, EPOLL_CTL_ADD, _outpipefd[0], &event) ==
            -1) {
          close(_outpipefd[0]);
          delete static_cast<EventData*>(event.data.ptr);
          _state = CGI_ERROR;
          return mapStateToConnectionStatus();
        }
      } catch (...) {
        throw;
      }
      _state = RUN_SCRIPT;
      return handleCGIRequest();
      break;

    case RUN_SCRIPT:
      try {
        _log.info("CGI: add fd to epoll: " + Utils::to_string(_outpipefd[0]));
        _pid = fork();
        if (_pid == -1) {
          throw ForkFailure("CGI: Failed to fork process");
        } else if (_pid > 0) {
          close(_inpipefd[1]);
          close(_outpipefd[0]);
          _state = READ_FROM_CGI;
        } else if (_pid == 0) {
          _log.info(
              "CGI: executing: " + _request.getURIComponents().scriptName +
              " with runtime: " + _runtime);
          close(_outpipefd[0]);
          if (dup2(_outpipefd[1], STDOUT_FILENO) == -1)
            throw ExecutorError(
                "CGI: dup2 failed: unable to redirect stdout to pipe");
          close(_outpipefd[1]);
          if (dup2(_inpipefd[0], STDIN_FILENO) == -1)
            throw ExecutorError(
                "CGI: dup2 failed: unable to redirect stdin to pipe");
          close(_inpipefd[0]);
          std::string postData = _request.getBody();
          std::size_t totalWritten = 0;
          int         trys = 0;  // Ajout d'un compteur de tentatives

          while (totalWritten < postData.size()) {
            ssize_t written =
                write(_inpipefd[1], postData.c_str() + totalWritten,
                      postData.size() - totalWritten);
            if (written == -1) {
              if (errno == EAGAIN || errno == EWOULDBLOCK) {
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
              } else {
                close(_inpipefd[1]);
                throw ExecutorError(
                    "CGI: write failed: unable to write POST data to pipe, "
                    "error: " +
                    std::string(strerror(errno)));
              }
            }
            trys = 0;
            totalWritten += written;
          }
          close(_inpipefd[1]);
          execve(_argv[0], &_argv[0], &_envp[0]);
          throw ExecutorError(
              "CGI: execve failed: unable to execute CGI script");
        }
      } catch (...) {
        _state = CGI_ERROR;
        throw;
      }
      _state = READ_FROM_CGI;
      break;

    case READ_FROM_CGI:
      try {
        char    buffer[248];
        ssize_t count = 0;
        pid_t   pidReturn = 0;
        bool    done = false;
        int     status = -1;

        close(_outpipefd[1]);
        _outpipefd[1] = -1;

        if (!_done &&
            (pidReturn = waitpid(_pid, &status,
                                 WNOHANG | WUNTRACED | WCONTINUED)) == -1) {
          _log.info("CGI: waitpid failed: " + std::string(strerror(errno)));
        }
        if (!_done && (pidReturn == 0 || WIFCONTINUED(status))) {
          _log.info("CGI: script is still running");
          _state = RUN_SCRIPT;
          return mapStateToConnectionStatus();
        }
        if (pidReturn > 0) {
          _done = true;
          if (WIFEXITED(status)) {
            int exitStatus = WEXITSTATUS(status);
            _log.info("CGI: script exited, status=" +
                      Utils::to_string(exitStatus));
            if (exitStatus != 0) {
              _log.warning("CGI: script finished with errors, exit status: " +
                           Utils::to_string(exitStatus));
            }
          }
          if (WIFSIGNALED(status))
            _log.error("CGI: script killed by signal: " +
                       Utils::to_string(WTERMSIG(status)));
          if (WIFSTOPPED(status))
            _log.error("CGI: script stopped by signal: " +
                       Utils::to_string(WSTOPSIG(status)));
        }

        count = read(_outpipefd[0], buffer, sizeof(buffer));
        if (!done && count == -1) {
          _log.warning("CGI: read failed: " + std::string(strerror(errno)));
          return mapStateToConnectionStatus();
        }
        if (count != -1) {
          _processOutputSize += count;
          _processOutput.append(buffer, count);
          // _log.info("CGI: read " + Utils::to_string(count) + " bytes");
          // _log.info("CGI: processOutputSize: " +
          //           Utils::to_string(_processOutputSize));
        }
        if (done) {
          _state = PROCESS_OUTPUT;
          return handleCGIRequest();
        }
      } catch (...) {
        _state = CGI_ERROR;
        throw;
      }
      _state = PROCESS_OUTPUT;
      break;

    case PROCESS_OUTPUT:
      try {
        _log.info("CGI: processOutput: " + _processOutput);
        _log.info("CGI: process done");
        std::size_t headerEndPos = _processOutput.find("\r\n\r\n");
        if (headerEndPos == std::string::npos)
          throw ExecutorError(
              "CGI: Invalid response: no header-body separator found");
        std::string headerPart = _processOutput.substr(0, headerEndPos);
        std::string bodyContent = _processOutput.substr(
            headerEndPos + 4);  // +4 to skip the "\r\n\r\n"

        std::map<std::string, std::string> headers =
            _parseOutputHeaders(headerPart);
        headers["Content-Length"] = Utils::to_string(bodyContent.size());
        _response.setHeaders(headers);
        _response.setBody(bodyContent);
      } catch (...) {
        _state = CGI_ERROR;
        throw;
      }
      _state = FINALIZE_RESPONSE;
      return handleCGIRequest();
      break;

    case FINALIZE_RESPONSE:
      try {
        std::map<std::string, std::string> headers =
            _parseOutputHeaders(_processOutput);
        headers["Content-Length"] =
            Utils::to_string(_response.getBody().size());
        _response.setHeaders(headers);

        if (_request.getHeader("Cache-Control") == "no-cache")
          _response.addHeader("Cache-Control", "no-cache");
        else {
          _response.addHeader(
              "Cache-Control",
              "public, max-age=" + Utils::to_string(CacheHandler::MAX_AGE));
          _cacheHandler.storeResponse(_request, _response);
        }
      } catch (...) {
        _state = CGI_ERROR;
        throw;
      }
      _state = DONE;
      break;

    case CGI_ERROR:
      return mapStateToConnectionStatus();

    case DONE:
      return mapStateToConnectionStatus();

    default:
      _log.error("CGI: Unexpected default state encountered");
      return ERROR;
  }

  return mapStateToConnectionStatus();
}

std::map<std::string, std::string> CGIHandler::_parseOutputHeaders(
    const std::string& headerPart) {
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
