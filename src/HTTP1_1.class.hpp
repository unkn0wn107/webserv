/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP1_1.class.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 12:51:06 by mchenava          #+#    #+#             */
/*   Updated: 2024/05/03 00:26:26 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP1_1_HPP
#define HTTP1_1_HPP

#include "HTTPProtocol.class.hpp"

class HTTP1_1 : public HTTPProtocol {
private:
    void _handleGetRequest(const Request* request, Response* response) const;

public:
    HTTP1_1();
    HTTP1_1(const HTTP1_1& other);
    HTTP1_1& operator=(const HTTP1_1& other);
    ~HTTP1_1();

    void processRequest(const Request* request, Response* response) const;
};

#endif // HTTP1_1_HPP
