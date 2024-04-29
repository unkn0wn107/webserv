#include "cgi_handler.h"
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <sstream>

CGIHandler::CGIHandler() {
}

CGIHandler::~CGIHandler() {
}

void CGIHandler::setupEnvironment(const HttpRequest& request) {
    // Clear previous environment variables
    envVariables.clear();

    // Convert HTTP headers to CGI environment variables
    headersToEnv(request.getHeaders());

    // Add REQUEST_METHOD
    std::string method;
    switch (request.getMethod()) {
        case GET: method = "GET"; break;
        case POST: method = "POST"; break;
        case DELETE: method = "DELETE"; break;
        default: method = "UNKNOWN"; break;
    }
    envVariables["REQUEST_METHOD"] = method;

    // Add QUERY_STRING and SCRIPT_NAME
    std::string uri = request.getUri();
    size_t pos = uri.find('?');
    if (pos != std::string::npos) {
        envVariables["QUERY_STRING"] = uri.substr(pos + 1);
        uri = uri.substr(0, pos);
    } else {
        envVariables["QUERY_STRING"] = "";
    }
    envVariables["SCRIPT_NAME"] = uri;

    // Add other CGI-specific environment variables
    envVariables["REMOTE_HOST"] = "localhost"; // Placeholder
    envVariables["CONTENT_LENGTH"] = std::to_string(request.getBody().length());
    envVariables["CONTENT_TYPE"] = request.getHeader("Content-Type");
}

void CGIHandler::headersToEnv(const std::map<std::string, std::string>& headers) {
    for (const auto& header : headers) {
        std::string envName = "HTTP_" + header.first;
        std::replace(envName.begin(), envName.end(), '-', '_');
        std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
        envVariables[envName] = header.second;
    }
}

bool CGIHandler::executeCGI(const HttpRequest& request, HttpResponse& response, const std::string& scriptPath) {
    setupEnvironment(request);

    std::string outputData;
    if (!createAndRunCGIProcess(scriptPath, request.getBody(), outputData)) {
        response.setStatusCode(INTERNAL_SERVER_ERROR);
        return false;
    }

    parseCGIOutput(outputData, response);
    return true;
}

bool CGIHandler::createAndRunCGIProcess(const std::string& scriptPath, const std::string& requestData, std::string& outputData) {
    int pipeIn[2], pipeOut[2];
    if (pipe(pipeIn) != 0 || pipe(pipeOut) != 0) {
        return false;
    }

    pid_t pid = fork();
    if (pid == -1) {
        return false;
    } else if (pid == 0) {
        // Child process
        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);
        close(pipeIn[1]);
        close(pipeOut[0]);

        char** envp = new char*[envVariables.size() + 1];
        int i = 0;
        for (const auto& env : envVariables) {
            std::string envEntry = env.first + "=" + env.second;
            envp[i++] = strdup(envEntry.c_str());
        }
        envp[i] = nullptr;

        execl(scriptPath.c_str(), scriptPath.c_str(), (char*)nullptr, envp);
        exit(1);
    } else {
        // Parent process
        close(pipeIn[0]);
        close(pipeOut[1]);

        write(pipeIn[1], requestData.c_str(), requestData.size());
        close(pipeIn[1]);

        char buffer[1024];
        ssize_t nread;
        while ((nread = read(pipeOut[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[nread] = '\0';
            outputData.append(buffer);
        }
        close(pipeOut[0]);

        int status;
        waitpid(pid, &status, 0);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
}

void CGIHandler::parseCGIOutput(const std::string& output, HttpResponse& response) {
    std::istringstream stream(output);
    std::string line;
    bool headerSection = true;

    while (std::getline(stream, line) && !line.empty()) {
        if (headerSection) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string headerName = line.substr(0, pos);
                std::string headerValue = line.substr(pos + 2);
                response.setHeader(headerName, headerValue);
            } else {
                headerSection = false; // Empty line, end of headers
            }
        } else {
            response.setBody(response.getBody() + line + "\n");
        }
    }
}
