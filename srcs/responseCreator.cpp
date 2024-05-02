#include "responseCreator.hpp"

/* Constructors */

ResponseCreator::ResponseCreator(void)
: _httpVersion("default"), _statusCode("default"), _reasonPhrase("default")
{
	return ;
}

ResponseCreator::ResponseCreator(ResponseCreator const &obj)
: _httpVersion(obj.getHTTPVersion()), _statusCode(obj.getStatusCode()), _reasonPhrase(obj.getReasonPhrase())
{
	return ;
}


/* Destructor */

ResponseCreator::~ResponseCreator(void)
{
	return ;
}


/* Copy Assignment Overload */

ResponseCreator	&ResponseCreator::operator=(ResponseCreator const &obj)
{
	this->_httpVersion = obj.getHTTPVersion();
	this->_statusCode = obj.getStatusCode();
	this->_reasonPhrase = obj.getReasonPhrase();
	return (*this);
}


/* Getters */

std::string	ResponseCreator::getHTTPVersion(void) const
{
	return (this->_httpVersion);
}

std::string	ResponseCreator::getStatusCode(void) const
{
	return (this->_statusCode);
}

std::string	ResponseCreator::getReasonPhrase(void) const
{
	return (this->_reasonPhrase);
}


/* Actions */

std::string	ResponseCreator::createStatusLine(void)
{
	this->_httpVersion = "HTTP/1.1";
	this->_statusCode = "200";
	this->_reasonPhrase = "OK";
	return (this->_httpVersion + " " + this->_statusCode + " " + this->_reasonPhrase + "\r\n");
}

std::string	ResponseCreator::createHeaderFields(void)
{
	return ("Content-length:0\r\n");
}
