#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <map>
#include <string>
#include "config.h"
#include "socket.h"
#include "http_request.h"
#include "http_response.h"
#include "cgi_handler.h"

class Server
{
public:
    Server(const std::string &configFilePath);
    ~Server();

    // Initialize the server with configuration
    bool initialize();

    // Start the server loop
    void run();

    // Stop the server
    void stop();

private:
    Config config;
    std::vector<Socket> sockets;
    std::map<int, HttpRequest> requests;   // Maps socket file descriptors to HTTP requests
    std::map<int, HttpResponse> responses; // Maps socket file descriptors to HTTP responses
    std::map<int, std::string> clientData; // Maps socket file descriptors to partial client data

    // Load server configurations
    bool loadServerConfig();

    // Setup listening sockets based on the configuration
    bool setupSockets();

    // Handle incoming connections
    void handleConnections();

    // Process incoming data from clients
    void processData(int sockfd);

    // Send responses to clients
    void sendResponses(int sockfd);

    // Handle CGI execution
    void handleCGI(int sockfd);

    // Utility to log server events
    void logEvent(const std::string &event);

    // Non-copyable and non-movable
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
};

#endif // SERVER_H
