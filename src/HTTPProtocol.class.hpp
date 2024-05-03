/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPProtocol.class.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:31:34 by mchenava          #+#    #+#             */
/*   Updated: 2024/05/03 00:27:46 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPROTOCOL_HPP
#define HTTPPROTOCOL_HPP

#include "Request.class.hpp"
#include "Response.class.hpp"
#include "Logger.class.hpp"

class HTTPProtocol {
protected:
        Logger& _log;
public:
    HTTPProtocol() : _log(Logger::getInstance()) {}
    virtual ~HTTPProtocol() {};

    // Méthode pour traiter une requête et produire une réponse
    virtual void processRequest(const Request* request, Response* response) const = 0;
};

#endif // HTTPPROTOCOL_HPP
