/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP1_1.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:51:31 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 17:07:23 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP1_1.class.hpp"

HTTP1_1::HTTP1_1() : HTTPProtocol() {}

HTTP1_1::~HTTP1_1() {}

HTTP1_1::HTTP1_1(const HTTP1_1& other) : HTTPProtocol() {
	*this = other;
}

HTTP1_1& HTTP1_1::operator=(const HTTP1_1& other) {
	*this = other;
	return *this;
}


void	HTTP1_1::processRequest(const Request& request, Response& response) const {
	// Exemple de traitement basique d'une requête GET
	if (request.getMethod() == "GET") {
		_handleGetRequest(request, response);
	} else {
		// Gérer d'autres méthodes HTTP ou renvoyer une erreur 405 (Méthode non autorisée)
		response.setStatusCode(405);
		response.setHeader("Content-Type", "text/plain");
		response.setBody("405 Method Not Allowed");
	}
}

void	HTTP1_1::_handleGetRequest(const Request& request, Response& response) const{
	// Ici, vous implémenteriez la logique pour gérer une requête GET
	// Par exemple, chercher une ressource dans le système de fichiers et la renvoyer
	// Ou renvoyer une erreur 404 si la ressource n'est pas trouvée

	// Ceci est un exemple simplifié renvoyant une réponse fixe
	(void)request;
	_log.info("GET request received");
	response.setStatusCode(200); // OK
	response.setHeader("Content-Type", "text/html");
	response.setBody("<html><body><h1>Hello, World!</h1></body></html>");
}

