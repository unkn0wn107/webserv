#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <fstream>
#include "config.h"
#include "http_request.h"
#include "http_response.h"

// Class to handle file operations for the HTTP server
class FileHandler
{
public:
    FileHandler();
    ~FileHandler();

    // Serve a file or directory based on the HTTP request and route configuration
    void handleRequest(const HttpRequest &request, HttpResponse &response, const RouteConfig &routeConfig);

    // Handle file upload based on the HTTP request and route configuration
    void handleFileUpload(const HttpRequest &request, HttpResponse &response, const RouteConfig &routeConfig);

private:
    // Check if the requested path is a directory
    bool isDirectory(const std::string &path);

    // List directory contents if directory listing is enabled
    std::string listDirectoryContents(const std::string &directoryPath);

    // Read file content into a string
    std::string readFile(const std::string &filePath);

    // Write content to a file
    bool writeFile(const std::string &filePath, const std::string &content);

    // Generate a default error page
    std::string generateDefaultErrorPage(HttpStatusCode statusCode);

    // Resolve the physical path from the route configuration and request URI
    std::string resolvePhysicalPath(const std::string &rootDirectory, const std::string &requestUri);
};

#endif // FILE_HANDLER_H
