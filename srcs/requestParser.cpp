#include "requestParser.hpp"

/* Constructors */

RequestParser::RequestParser(void)
: _method("default"), _requestTarget("default"), _httpVersion("default")
{
	return ;
}

RequestParser::RequestParser(RequestParser const &obj)
: _method(obj.getMethod()), _requestTarget(obj.getRequestTarget()), _httpVersion(obj.getHTTPVersion())
{
	return ;
}


/* Destructor */

RequestParser::~RequestParser(void)
{
	return ;
}


/* Copy Assignment Overload */

RequestParser	&RequestParser::operator=(RequestParser const &obj)
{
	this->_method = obj.getMethod();
	this->_requestTarget = obj.getRequestTarget();
	this->_httpVersion = obj.getHTTPVersion();
	return (*this);
}


/* Getters */

std::string	RequestParser::getMethod(void) const
{
	return (this->_method);
}

std::string	RequestParser::getRequestTarget(void) const
{
	return (this->_requestTarget);
}

std::string	RequestParser::getHTTPVersion(void) const
{
	return (this->_httpVersion);
}


/* Actions */

bool	RequestParser::parseRequestLine(std::string line)
{
	this->_method = getToken(&line);
	this->_requestTarget = getToken(&line);
	if (this->_method.empty() || this->_requestTarget.empty())
		return (false);
	if (line.find(' ') != std::string::npos || line[0] == '\0')
		return (false);
	this->_httpVersion = line;
	return (true);
}

std::string	RequestParser::getToken(std::string *line)
{
	size_t		pos;
	std::string	token;

	pos = line->find(' ');
	if (pos == std::string::npos || pos == 0)
		return ("");
	token = line->substr(0, pos);
	*line = line->substr(pos + 1);
	return (token);
}

bool	RequestParser::parseMethodToken(void)
{
	if (this->_method != "GET" && this->_method != "POST" && this->_method != "DELETE" && this->_method != "HEAD")
		return (false);
	return (true);
}

bool	RequestParser::parseHTTPVersionToken(void)
{
	size_t	pointCount = 0;

	if (this->_httpVersion.compare(0, 5, "HTTP/"))
		return (false);
	for (size_t pos = 5; pos < this->_httpVersion.size(); pos++)
	{
		if (this->_httpVersion[pos] == '.')
		{
			if (this->_httpVersion[pos + 1] == '\0')
				return (false);
			pointCount++;
		}
		else if (!isdigit(this->_httpVersion[pos]))
			return (false);
	}
	if (pointCount != 1)
		return (false);
	return (true);
}

bool	RequestParser::httpVersionSupported(void) 
{
	if (this->_httpVersion.compare(0, 7, "HTTP/1."))
		return (false);
	return (true);
}

bool	RequestParser::parseHeaderField(std::string line)
{
	size_t		pos;
	std::string	fieldName;
	std::string	fieldValue;

	pos = line.find(':');
	if (pos == std::string::npos || pos == 0)
		return (false);
	fieldName = line.substr(0, pos);
	fieldValue = line.substr(pos + 1);
	if (!parseFieldName(fieldName))
		return (false);
	cleanFieldValue(&fieldValue);
	this->_headers.insert(std::make_pair(fieldName, fieldValue));
	for (std::map<std::string, std::string>::iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
	{
		std::cout << it->first << std::endl;
		std::cout << it->second << std::endl;
	}
	return (true);
}

bool	RequestParser::parseFieldName(std::string fieldName)
{
	if (fieldName.find_first_of("\"(),/:;<=>?@[\\]{} \t\n\v\f\r") != std::string::npos)
		return (false);
	return (true);
}

void	RequestParser::cleanFieldValue(std::string *fieldValue)
{
	size_t	posBegin;
	size_t	posEnd;
	size_t	len;

	posBegin = 0;
	while (isspace((*fieldValue)[posBegin]))
		posBegin++;
	posEnd = fieldValue->size() - 1;
	while (posEnd != 0 && isspace((*fieldValue)[posEnd]))
		posEnd--;
	len = posEnd - posBegin + 1;
	*fieldValue = fieldValue->substr(posBegin, len);
}

	
