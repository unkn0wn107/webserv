#ifndef HTTP2_H
#define HTTP2_H

#include "HTTPProtocol.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

class HTTP2 : public HTTPProtocol
{
public:
	HTTP2();
	virtual ~HTTP2();

	// Parses the HTTP/2 request from a string format into an HTTPRequest object
	virtual HTTPRequest parseRequest(const std::string &requestString);

	// Creates a new HTTPResponse object for responding to an HTTP/2 request
	virtual HTTPResponse createResponse();

	// Processes the HTTPRequest object and returns an HTTPResponse object
	virtual HTTPResponse processRequest(const HTTPRequest &request);
};

#endif // HTTP2_H
