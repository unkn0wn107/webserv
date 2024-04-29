#include "http_request.h"
#include <sstream>
#include <algorithm>

HttpRequest::HttpRequest(int clientFd) : clientFd(clientFd), parsed(false), validRequest(false) {
    buffer.reserve(4096); // Reserve 4KB initially for the request buffer
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::receiveRequest() {
    char tempBuffer[1024];
    ssize_t bytesRead = 0;

    while ((bytesRead = read(clientFd, tempBuffer, sizeof(tempBuffer))) > 0) {
        buffer.append(tempBuffer, bytesRead);
    }

    if (bytesRead < 0) {
        return false; // Error occurred during read
    }

    if (bytesRead == 0 && buffer.empty()) {
        return false; // No data read, possibly client disconnected
    }

    return parseRequest();
}

bool HttpRequest::parseRequest() {
    std::istringstream requestStream(buffer);
    std::string requestLine;
    if (!std::getline(requestStream, requestLine)) {
        return false;
    }

    std::istringstream lineStream(requestLine);
    lineStream >> method >> uri >> version;

    // Normalize method to uppercase
    std::transform(method.begin(), method.end(), method.begin(), ::toupper);

    // Parse headers
    std::string headerLine;
    while (std::getline(requestStream, headerLine) && headerLine != "\r") {
        size_t separator = headerLine.find(':');
        if (separator != std::string::npos) {
            std::string key = headerLine.substr(0, separator);
            std::string value = headerLine.substr(separator + 2, headerLine.length() - separator - 3); // Skip ": " and "\r"
            headers[key] = value;
        }
    }

    // Check if the request method and headers are valid
    if (method.empty() || uri.empty() || version.empty()) {
        return false;
    }

    parsed = true;
    validRequest = true;
    return true;
}

const std::string& HttpRequest::getMethod() const {
    return method;
}

const std::string& HttpRequest::getUri() const {
    return uri;
}

const std::string& HttpRequest::getVersion() const {
    return version;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders() const {
    return headers;
}

bool HttpRequest::isValid() const {
    return validRequest;
}

int HttpRequest::getClientFd() const {
    return clientFd;
}
