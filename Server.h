#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include "ConnectionHandler.h"
#include "ConfigLoader.h"
#include "ErrorHandler.h"

class Server
{
public:
	static Server &getInstance();
	void start();
	ConnectionHandler *acceptConnection();

private:
	Server();
	Server(const Server &);			   // Prevent copy-construction
	Server &operator=(const Server &); // Prevent assignment
	~Server();

	void setupServer();
	void handleConnections();

	static Server *instance;
	ConfigLoader &config;
	ErrorHandler errorHandler;
	std::vector<ConnectionHandler *> connections;
	int epoll_fd;
};

#endif // SERVER_H
