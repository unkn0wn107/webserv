/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmohin <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/29 21:33:35 by lmohin            #+#    #+#             */
/*   Updated: 2024/05/28 04:06:01 by lmohin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "requestParser.hpp"
#include "responseCreator.hpp"

int	main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cout << "Wrong number of arguments!" << std::endl;
		return (1);
	}
	
	RequestParser	parser;
	if (!parser.parseRequestLine(argv[1]))
	{
		// 400 Bad Request
		std::cout << "Bad Request" << std::endl;
		return (1);
	}
	/**/
		std::cout << "Method: " << parser.getMethod() << std::endl;
		std::cout << "Request Target: " << parser.getRequestTarget() << std::endl;
		std::cout << "HTTP Version: " << parser.getHTTPVersion() << std::endl;
	/**/
	if (!parser.parseMethodToken())
	{
		// 501 Not Implemented
		std::cout << "Method Not Implemented" << std::endl;
		return (1);
	}
	std::cout << "correct method" << std::endl;
	if (!parser.parseHTTPVersionToken())
	{
		// 400 Bad Request
		std::cout << "Bad Request" << std::endl;
		return (1);
	}
	if (!parser.httpVersionSupported())
	{
		// 505 HTTP Version Not Supported
		std::cout << "HTTP Version Not Supported" << std::endl;
		return (1);
	}
	for (int i = 2; i != argc; i++)
	{
		if (!parser.parseHeaderField(argv[i]))
		{
			// 400 Bad Request
			std::cout << "Bad Request" << std::endl;
		}
	}
	ResponseCreator	creator;
	std::cout << std::endl;
	std::cout << creator.createStatusLine();
	std::cout << creator.createHeaderFields();
	std::cout << "\r\n";
	// std::cout << creator.createBodyField();
	std::cout << "\r\n";
	return (0);
}
