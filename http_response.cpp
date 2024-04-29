#include "http_response.h"
#include <sstream>
#include <fstream>
#include <iostream>

HttpResponse::HttpResponse() : statusCode(200), statusMessage("OK") {}

HttpResponse::~HttpResponse() {}

void HttpResponse::setStatusCode(int code) {
    statusCode = code;
    statusMessage = getDefaultStatusMessage(code);
}

void HttpResponse::setStatusMessage(const std::string& message) {
    statusMessage = message;
}

void HttpResponse::addHeader(const std::string& name, const std::string& value) {
    headers[name] = value;
}

void HttpResponse::setBody(const std::string& bodyContent) {
    body = bodyContent;
}

std::string HttpResponse::generateResponse() const {
    std::ostringstream responseStream;

    // Start line
    responseStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";

    // Headers
    for (const auto& header : headers) {
        responseStream << header.first << ": " << header.second << "\r\n";
    }

    // End of headers
    responseStream << "\r\n";

    // Body
    responseStream << body;

    return responseStream.str();
}

std::string HttpResponse::getDefaultStatusMessage(int statusCode) {
    switch (statusCode) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default: return "Unknown";
    }
}

void HttpResponse::sendResponse(int clientSocket) {
    std::string response = generateResponse();
    send(clientSocket, response.c_str(), response.size(), 0);
}

void HttpResponse::loadDefaultErrorPage(const std::string& filePath) {
    std::ifstream fileStream(filePath.c_str(), std::ifstream::in);
    if (fileStream) {
        std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
        setBody(content);
    } else {
        setBody("<html><body><h1>Error " + std::to_string(statusCode) + " - " + statusMessage + "</h1></body></html>");
    }
}
