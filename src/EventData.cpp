/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventData.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 19:31:18 by agaley            #+#    #+#             */
/*   Updated: 2024/07/23 02:06:00 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventData.hpp"

EventData::EventData(int fd, ConnectionHandler* ptr, pthread_mutex_t& requestTimesMutex, pid_t threadId, bool isListening, bool recordTime, bool opened)
    : fd(fd), handler(ptr), requestTimesMutex(requestTimesMutex), threadId(threadId), isListening(isListening), recordTime(recordTime), opened(opened) {}
