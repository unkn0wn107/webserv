#ifndef COMMON_HPP
#define COMMON_HPP

#include <ctime>

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3

#define LOG_LEVEL LOG_LEVEL_INFO
#define LOG_FILE_PATH "./logs/"
#define LOG_FILE_NAME "webserv"
#define LOG_FILE_EXT ".log"

#define MAX_TRIES 10
#define TIMEOUT 10
#define BUFFER_SIZE 16384
#define MAX_EVENTS 1024
#define SHUTDOWN_DELAY 200000

enum ConnectionStatus { READING, EXECUTING, CACHE_WAITING, SENDING, CLOSED };

enum CGIState {
  NONE,
  REGISTER_SCRIPT_FD,
  RUN_SCRIPT,
  SCRIPT_RUNNING,
  READ_FROM_CGI,
  PROCESS_OUTPUT,
  DONE,
  CGI_ERROR,
};

enum CacheStatus {
  CACHE_CURRENTLY_BUILDING,
  CACHE_FOUND,
  CACHE_NOT_FOUND,
};

#endif
