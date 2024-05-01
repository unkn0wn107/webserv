/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmohin <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/29 21:33:35 by lmohin            #+#    #+#             */
/*   Updated: 2024/05/01 18:40:53 by lmohin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "requestParser.hpp"

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Wrong number of arguments!" << std::endl;
		return (1);
	}
	
	RequestParser	parser;
	if (!parser.parseRequestLine(argv[1]))
	{
		// 400 Bad Request or 301 Moved Permanently
		std::cout << "Bad Request" << std::endl;
		return (1);
	}
	std::cout << "Method: " << parser.getMethod() << std::endl;
	std::cout << "Request Target: " << parser.getRequestTarget() << std::endl;
	std::cout << "HTTP Version: " << parser.getHTTPVersion() << std::endl;
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
	return (0);
}
