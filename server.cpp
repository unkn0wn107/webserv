#include "server.h"
#include "config.h"
#include "socket.h"
#include "http_request.h"
#include "http_response.h"
#include "cgi_handler.h"
#include "file_handler.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <poll.h>
#include <signal.h>

Server::Server(const std::string& configFilePath) : config(configFilePath) {
    if (!config.loadConfiguration()) {
        throw std::runtime_error("Failed to load configuration.");
    }
}

Server::~Server() {
    for (auto& sock : sockets) {
        sock.closeSocket();
    }
}

bool Server::initialize() {
    const auto& serverConfigs = config.getAllServerConfigs();
    for (const auto& serverConfig : serverConfigs) {
        Socket socket;
        if (!socket.init(serverConfig.port, serverConfig.host)) {
            std::cerr << "Failed to initialize socket for " << serverConfig.host << ":" << serverConfig.port << std::endl;
            continue;
        }
        socket.setNonBlocking();
        if (!socket.listenOnSocket()) {
            std::cerr << "Failed to listen on socket for " << serverConfig.host << ":" << serverConfig.port << std::endl;
            continue;
        }
        sockets.push_back(std::move(socket));
    }
    return !sockets.empty();
}

void Server::run() {
    std::vector<struct pollfd> fds;
    for (auto& socket : sockets) {
        struct pollfd pfd;
        pfd.fd = socket.getSocketFd();
        pfd.events = POLLIN;
        fds.push_back(pfd);
    }

    while (true) {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret < 0) {
            std::cerr << "Poll error" << std::endl;
            break;
        }

        for (auto& pfd : fds) {
            if (pfd.revents & POLLIN) {
                Socket& socket = findSocketByFd(pfd.fd);
                int clientFd = socket.acceptConnection();
                if (clientFd < 0) {
                    continue;
                }

                handleClient(clientFd);
            }
        }
    }
}

void Server::handleClient(int clientFd) {
    HttpRequest request;
    HttpResponse response;
    if (!request.parseRequest(clientFd)) {
        response.sendErrorResponse(clientFd, 400); // Bad Request
        return;
    }

    const RouteConfig& routeConfig = config.getRouteConfig(request.getUrl());
    if (std::find(routeConfig.accepted_methods.begin(), routeConfig.accepted_methods.end(), request.getMethod()) == routeConfig.accepted_methods.end()) {
        response.sendErrorResponse(clientFd, 405); // Method Not Allowed
        return;
    }

    if (!routeConfig.redirection_target.empty()) {
        response.sendRedirectResponse(clientFd, routeConfig.redirection_target);
        return;
    }

    if (request.getMethod() == "GET") {
        FileHandler::handleGetRequest(clientFd, request, response, routeConfig);
    } else if (request.getMethod() == "POST") {
        FileHandler::handlePostRequest(clientFd, request, response, routeConfig);
    } else if (request.getMethod() == "DELETE") {
        FileHandler::handleDeleteRequest(clientFd, request, response, routeConfig);
    }
}

Socket& Server::findSocketByFd(int fd) {
    for (auto& socket : sockets) {
        if (socket.getSocketFd() == fd) {
            return socket;
        }
    }
    throw std::runtime_error("Socket not found for file descriptor.");
}
