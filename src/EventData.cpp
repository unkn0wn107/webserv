/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventData.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 19:31:18 by agaley            #+#    #+#             */
/*   Updated: 2024/07/04 20:02:54 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "EventData.hpp"

EventData::EventData(int                fd,
                     ConnectionHandler* ptr,
                     pid_t              threadId,
                     bool               isListening)
    : fd(fd), handler(ptr), threadId(threadId), isListening(isListening) {}
