/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventData.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 19:31:18 by agaley            #+#    #+#             */
/*   Updated: 2024/07/23 00:02:55 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventData.hpp"

EventData::EventData(int fd, ConnectionHandler* ptr, pid_t threadId, bool isListening, bool recordTime, bool opened)
    : fd(fd), handler(ptr), threadId(threadId), isListening(isListening), recordTime(recordTime), opened(opened) {}
