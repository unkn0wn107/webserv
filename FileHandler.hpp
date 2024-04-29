#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "FileManager.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Handler.hpp"

class FileHandler : public Handler {
 private:
  FileManager& fileManager;

 public:
  FileHandler(FileManager& fm) : fileManager(fm) {}

  virtual HTTPResponse processRequest(const HTTPRequest& request) {
    HTTPResponse response;
    try {
      std::string path = fileManager.resolvePath(request.getUrl());
      std::string content = fileManager.readFile(path);
      response.setBody(content);
      response.setStatusCode(200);
      response.setHeaders({{"Content-Type", fileManager.getMimeType(path)},
                           {"Content-Length", std::to_string(content.size())}});
    } catch (const std::exception& e) {
      response.setStatusCode(404);
      response.setBody("404 Not Found");
    }
    return response;
  }
};

#endif
