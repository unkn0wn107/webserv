#include "ConnectionHandler.hpp"
#include "CGIHandler.hpp"
#include "FileHandler.hpp"
#include "Server.hpp"

ConnectionHandler::ConnectionHandler(Server& server, int socket)
    : _server(server), _socket(socket) {}

ConnectionHandler::~ConnectionHandler() {
  close(_socket);
}

void ConnectionHandler::process() {
  char    buffer[1024];
  ssize_t bytes_read = recv(_socket, buffer, sizeof(buffer), 0);
  if (bytes_read <= 0) {
    // Handle error or close connection
    return;
  }

  try {
    _request.parse(buffer);
  } catch (const std::exception& e) {
    _response.setStatusCode(HTTPResponse::BAD_REQUEST);
    _response.setBody("Error parsing request: " + std::string(e.what()));
    sendResponse();
    return;
  }

  std::string url = _request.getUrl();
  std::string extension = url.substr(url.find_last_of('.'));

  if (CGIHandler::isScript(url))
    _response = CGIHandler::processRequest(_request);
  else
    _response = FileHandler::processRequest(_request);
  sendResponse();
}

void ConnectionHandler::sendResponse() {
  std::string response = _response.generate();
  send(_socket, response.c_str(), response.size(), 0);
}
