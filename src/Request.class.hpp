/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.class.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:39:27 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 16:36:09 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include "Logger.class.hpp"

class Request {
public:
    Request();
    Request(const std::string& rawRequest);
    Request(const Request& other);
    Request& operator=(const Request& other);
    ~Request();

    std::string getMethod() const;
    std::string getURI() const;
    std::string getHeader(const std::string& key) const;
    std::string getBody() const;
	bool isEmpty() const;
    void parse(std::string buffer, int bytesReceived);

private:
    Logger& _log;
    std::string _method;
    std::string _uri;
    std::map<std::string, std::string> _headers;
    std::string _body;

};

#endif // REQUEST_HPP
