#include "file_handler.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

FileHandler::FileHandler() {}

FileHandler::~FileHandler() {}

bool FileHandler::serveFile(const HttpRequest& request, HttpResponse& response, const RouteConfig& routeConfig) {
    std::string filePath = constructFilePath(request.getUri(), routeConfig);

    // Check if the path is a directory
    struct stat pathStat;
    if (stat(filePath.c_str(), &pathStat) == 0) {
        if (S_ISDIR(pathStat.st_mode)) {
            if (routeConfig.directory_listing_enabled) {
                return listDirectoryContents(filePath, response);
            } else if (!routeConfig.default_file.empty()) {
                filePath = filePath + "/" + routeConfig.default_file;
            } else {
                response.setStatusCode(NOT_FOUND);
                return false;
            }
        }
    }

    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        response.setStatusCode(NOT_FOUND);
        return false;
    }

    std::ostringstream fileContents;
    fileContents << file.rdbuf();
    file.close();

    response.setStatusCode(OK);
    response.setBody(fileContents.str());
    response.setHeader("Content-Type", "text/html"); // Simplified content type handling
    response.setHeader("Content-Length", std::to_string(fileContents.str().size()));

    return true;
}

bool FileHandler::listDirectoryContents(const std::string& directoryPath, HttpResponse& response) {
    // Directory listing is not implemented in this example for simplicity
    response.setStatusCode(NOT_IMPLEMENTED);
    return false;
}

std::string FileHandler::constructFilePath(const std::string& uri, const RouteConfig& routeConfig) {
    return routeConfig.root_directory + uri;
}

bool FileHandler::handleFileUpload(const HttpRequest& request, HttpResponse& response, const RouteConfig& routeConfig) {
    if (request.getMethod() != POST) {
        response.setStatusCode(METHOD_NOT_ALLOWED);
        return false;
    }

    std::string uploadPath = routeConfig.upload_path + extractFileName(request.getUri());
    std::ofstream outputFile(uploadPath.c_str(), std::ios::binary);
    if (!outputFile.is_open()) {
        response.setStatusCode(INTERNAL_SERVER_ERROR);
        return false;
    }

    outputFile.write(request.getBody().c_str(), request.getBody().size());
    outputFile.close();

    response.setStatusCode(CREATED);
    return true;
}

std::string FileHandler::extractFileName(const std::string& uri) {
    size_t lastSlashPos = uri.find_last_of('/');
    return uri.substr(lastSlashPos + 1);
}
