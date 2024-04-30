#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "FileManager.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Handler.hpp"

class FileHandler : public Handler {
 private:
  FileHandler();
  ~FileHandler();

 public:
  static HTTPResponse processRequest(const HTTPRequest& request);
};

#endif
