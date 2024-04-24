/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.class.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:40:03 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 16:36:04 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include "Logger.class.hpp"

class Response {
public:
    Response();
    Response(const Response& other);
    Response& operator=(const Response& other);
    ~Response();

    void setStatusCode(int code);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    std::string generate() const;

private:
    Logger& _log;
    int _statusCode;
    std::map<std::string, std::string> _headers;
    std::string _body;
};

#endif // RESPONSE_HPP
