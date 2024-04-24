/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:30:53 by mchenava          #+#    #+#             */
/*   Updated: 2024/04/24 15:20:17 by mchenava         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Server.class.hpp"
#include "ServerException.class.hpp"

int main() {
    try {
        Server server("configFilePath");
        server.run();
    } catch (const ServerException& e) {
        std::cerr << "Critical server error: " << e.what() << std::endl;
        // Décider de la suite, par exemple redémarrer le serveur ou simplement enregistrer l'erreur
    }
    return 0;
}
