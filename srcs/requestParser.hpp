#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

# include <string>
# include <iostream>
# include <map>

class RequestParser 
{
	public :
		
		RequestParser(void);
		RequestParser(RequestParser const &obj);
		
		~RequestParser(void);

		RequestParser	&operator=(RequestParser const &obj);

		std::string	getMethod(void) const;
		std::string	getRequestTarget(void) const;
		std::string	getHTTPVersion(void) const;

		bool	parseRequestLine(std::string line);
		bool	parseMethodToken(void);
		bool	parseHTTPVersionToken(void);
		bool	httpVersionSupported(void);

		bool	parseHeaderField(std::string line);
		bool	parseFieldName(std::string line);
		void	cleanFieldValue(std::string *fieldValue);

	private :

		std::string	getToken(std::string *line);

		std::string				_method;
		std::string				_requestTarget;
		std::string				_httpVersion;
		std::map<std::string, std::string>	_headers;
};

# endif
