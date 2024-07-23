/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventData.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 19:31:18 by agaley            #+#    #+#             */
/*   Updated: 2024/07/23 16:11:04 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "EventData.hpp"

EventData::EventData(int newFd, ConnectionHandler* ptr, pid_t newThreadId, bool newIsListening, bool newRecordTime, bool newOpened) {
        pthread_mutex_init(&mutex, NULL);
        setFd(newFd);
        setHandler(ptr);
        setThreadId(newThreadId);
        setIsListening(newIsListening);
        setRecordTime(newRecordTime);
        setStartTime(0);
        setOpened(newOpened);
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
    pthread_mutex_lock(&mutex);
    fd = newFd;
    pthread_mutex_unlock(&mutex);
}
void               EventData::setHandler(ConnectionHandler* newHandler){
    pthread_mutex_lock(&mutex);
    handler = newHandler;
    pthread_mutex_unlock(&mutex);
}
void               EventData::setThreadId(pid_t id){
    pthread_mutex_lock(&mutex);
    threadId = id;
    pthread_mutex_unlock(&mutex);
}
void               EventData::setIsListening(bool newIsListening){
    pthread_mutex_lock(&mutex);
    isListening = newIsListening;
    pthread_mutex_unlock(&mutex);
}
void               EventData::setRecordTime(bool record){
    pthread_mutex_lock(&mutex);
    recordTime = record;
    pthread_mutex_unlock(&mutex);
}
void               EventData::setStartTime(time_t time){
    pthread_mutex_lock(&mutex);
    startTime = time;
    pthread_mutex_unlock(&mutex);
}
void               EventData::setOpened(bool newOpened){
    pthread_mutex_lock(&mutex);
    opened = newOpened;
    std::cerr << "set opened" << std::endl;
    pthread_mutex_unlock(&mutex);
}

EventData::~EventData() {
    pthread_mutex_unlock(&mutex);
    std::cerr << "destroying" << std::endl;
    pthread_mutex_destroy(&mutex);
}
