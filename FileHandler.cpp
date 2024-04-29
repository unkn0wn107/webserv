#include "FileHandler.h"
#include "FileManager.h"
#include "HTTPResponse.h"
#include "HTTPRequest.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>

FileHandler::FileHandler() {
    fileManager = new FileManager();
}

FileHandler::~FileHandler() {
    delete fileManager;
}

HTTPResponse FileHandler::processRequest(const HTTPRequest& request) {
    HTTPResponse response;
    std::string path = fileManager->resolvePath(request.getUrl());
    struct stat path_stat;
    stat(path.c_str(), &path_stat);

    // Check if the path is a directory
    if (S_ISDIR(path_stat.st_mode)) {
        // Handle directory request
        std::string indexFilePath = path + "/index.html";
        if (fileManager->fileExists(indexFilePath)) {
            response.setBody(fileManager->readFile(indexFilePath));
            response.setStatusCode(200);
        } else {
            // Directory listing is turned off, return 403 Forbidden
            response.setStatusCode(403);
            response.setBody("403 Forbidden: Directory listing is disabled.");
        }
    } else if (fileManager->fileExists(path)) {
        // Handle file request
        response.setBody(fileManager->readFile(path));
        response.setStatusCode(200);
    } else {
        // File not found
        response.setStatusCode(404);
        response.setBody("404 Not Found: The requested resource was not found on this server.");
    }

    // Set common headers
    response.setHeaders({
        {"Content-Type", "text/html"},
        {"Content-Length", std::to_string(response.getBody().length())}
    });

    return response;
}
