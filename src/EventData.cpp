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
#include "EventData.hpp"

EventData::EventData(int newFd, ConnectionHandler* ptr, pid_t newThreadId, bool newIsListening, bool newRecordTime, bool newOpened) {
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_lock(&mutex);
        fd = newFd;
        handler = ptr;
        threadId = newThreadId;
        isListening = newIsListening;
        recordTime = newRecordTime;
        startTime = 0;
        opened = newOpened;
        pthread_mutex_unlock(&mutex);
    }

EventData::~EventData() {
    pthread_mutex_lock(&mutex);
    handler = NULL;
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
}

int                EventData::getFd(){
    pthread_mutex_lock(&mutex);
    int ret = fd;
    pthread_mutex_unlock(&mutex);
    return ret;
}
ConnectionHandler* EventData::getHandler(){
    pthread_mutex_lock(&mutex);
    ConnectionHandler* ret = handler;
    pthread_mutex_unlock(&mutex);
    return ret;
}
pid_t              EventData::getThreadId(){
    pthread_mutex_lock(&mutex);
    pid_t ret = threadId;
    pthread_mutex_unlock(&mutex);
    return ret;
}
bool               EventData::getIsListening(){
    pthread_mutex_lock(&mutex);
    bool ret = isListening;
    pthread_mutex_unlock(&mutex);
    return ret;
}
bool               EventData::getRecordTime(){
    pthread_mutex_lock(&mutex);
    bool ret = recordTime;
    pthread_mutex_unlock(&mutex);
    return ret;
}
time_t             EventData::getStartTime(){
    pthread_mutex_lock(&mutex);
    time_t ret = startTime;
    pthread_mutex_unlock(&mutex);
    return ret;
}
bool               EventData::getOpened(){
    pthread_mutex_lock(&mutex);
    bool ret = opened;
    pthread_mutex_unlock(&mutex);
    return ret;
}

void               EventData::setFd(int newFd){
    fd = newFd;
}
void               EventData::setHandler(ConnectionHandler* newHandler){
    handler = newHandler;
}
void               EventData::setThreadId(pid_t id){
    threadId = id;
}
void               EventData::setIsListening(bool newIsListening){
    isListening = newIsListening;
}
void               EventData::setRecordTime(bool record){
    recordTime = record;
}
void               EventData::setStartTime(time_t time){
    startTime = time;
}
void               EventData::setOpened(bool newOpened){
    opened = newOpened;
}
