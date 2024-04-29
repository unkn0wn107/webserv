#include "ConnectionHandler.hpp"

ConnectionHandler::ConnectionHandler(Socket socket) : _socket(socket) {}

ConnectionHandler::ConnectionHandler(const ConnectionHandler& other) {
  _socket = other._socket;
}

ConnectionHandler& ConnectionHandler::operator=(
    const ConnectionHandler& other) {
  _socket = other._socket;
  return *this;
}

ConnectionHandler::~ConnectionHandler() {
  delete _socket;
}

void ConnectionHandler::handleConnection(Socket clientSocket) {
  // Process incoming requests and send responses for a single client connection

  // Set up HTTP request and response objects
  HTTPRequest  request;
  HTTPResponse response;

  // Parse incoming request from client
  try {
    request.parse(clientSocket);
  } catch (const std::exception& e) {
    // Handle parsing errors
    response.setStatusCode(HTTPResponse::BAD_REQUEST);
    response.setBody("Error parsing request: " + std::string(e.what()));
    response.send(clientSocket);
    return;
  }

  // Route request to appropriate handler
  Router                   router;
  std::unique_ptr<Handler> handler = router.route(request);

  // Process request and generate response
  try {
    handler->processRequest(request, response);
  } catch (const std::exception& e) {
    // Handle processing errors
    response.setStatusCode(HTTPResponse::INTERNAL_SERVER_ERROR);
    response.setBody("Error processing request: " + std::string(e.what()));
  }

  // Send response back to client
  response.send(clientSocket);

  // Log connection details
  Logger::log("Connection closed: " + clientSocket.getRemoteAddress() + ":" +
              std::to_string(clientSocket.getRemotePort()));
}
