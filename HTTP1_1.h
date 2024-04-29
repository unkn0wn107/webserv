#ifndef HTTP1_1_H
#define HTTP1_1_H

#include "HTTPProtocol.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include <string>

class HTTP1_1 : public HTTPProtocol
{
public:
	HTTP1_1();
	virtual ~HTTP1_1();

	// Parses the incoming request string and returns an HTTPRequest object
	virtual HTTPRequest parseRequest(const std::string &requestString);

	// Creates an HTTPResponse object to be used for sending back to client
	virtual HTTPResponse createResponse();

	// Processes the HTTPRequest object and returns an HTTPResponse object
	virtual HTTPResponse processRequest(const HTTPRequest &request);
};

#endif // HTTP1_1_H
