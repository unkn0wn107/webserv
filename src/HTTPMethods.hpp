/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPMethods.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 11:56:13 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/11 12:45:01 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPMETHODS_HPP
#define HTTPMETHODS_HPP

#include <string>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "VirtualServer.hpp"
#include "Logger.hpp"

class VirtualServer;

class HTTPMethods {
	private:
		VirtualServer& 	_server;
		Logger&					_log;

		HTTPResponse*		_handleGetRequest(HTTPRequest& request);
		HTTPResponse*		_handlePostRequest(HTTPRequest& request);
		HTTPResponse* 	_autoindex(const std::string& path, LocationConfig& location);
		std::string 		_generateDirectoryListing(const std::string& path);
		std::string 		_getPath(const std::string& uri, LocationConfig& location);
	public:
		HTTPMethods(VirtualServer& server);
		~HTTPMethods();
		HTTPResponse* handleRequest(HTTPRequest& request);
};

#endif
