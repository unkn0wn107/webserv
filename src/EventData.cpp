/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventData.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 19:31:18 by agaley            #+#    #+#             */
/*   Updated: 2024/07/23 19:18:27 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "EventData.hpp"

EventData::EventData(int newFd, ConnectionHandler* ptr, bool newIsListening, bool newRecordTime, bool newOpened)
            :fd(newFd), handler(ptr), isListening(newIsListening), recordTime(newRecordTime), opened(newOpened) {}

EventData::~EventData() {
}
