#ifndef RESPONSECREATOR_HPP
# define RESPONSECREATOR_HPP

# include <string>
# include <iostream>

class ResponseCreator
{
	public :
		
		ResponseCreator(void);
		ResponseCreator(ResponseCreator const &obj);
		
		~ResponseCreator(void);

		ResponseCreator	&operator=(ResponseCreator const &obj);

		std::string	getHTTPVersion(void) const;
		std::string	getStatusCode(void) const;
		std::string	getReasonPhrase(void) const;

		std::string	createStatusLine(void);
		std::string	createHeaderFields(void);

	private :

		std::string	_httpVersion;
		std::string	_statusCode;
		std::string	_reasonPhrase;
};

# endif
