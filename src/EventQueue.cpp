/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventQueue.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/27 16:00:52 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/27 16:10:05 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EventQueue.hpp"

EventQueue::EventQueue() {
    pthread_mutex_init(&_mutex, NULL);
    pthread_cond_init(&_cond, NULL);
}

EventQueue::~EventQueue() {
    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&_cond);
}

void EventQueue::push(const struct epoll_event& event) {
    pthread_mutex_lock(&_mutex);
    _queue.push(event);
    pthread_cond_signal(&_cond);
    pthread_mutex_unlock(&_mutex);
}

bool EventQueue::pop(struct epoll_event& event) {
    pthread_mutex_lock(&_mutex);
    while (_queue.empty()) {
        pthread_cond_wait(&_cond, &_mutex);
    }
    event = _queue.front();
    _queue.pop();
    pthread_mutex_unlock(&_mutex);
    return true;
}

bool EventQueue::try_pop(struct epoll_event& event) {
    pthread_mutex_lock(&_mutex);
    if (_queue.empty()) {
        pthread_mutex_unlock(&_mutex);
        return false;
    }
    event = _queue.front();
    _queue.pop();
    pthread_mutex_unlock(&_mutex);
    return true;
}

bool EventQueue::empty() const {
    pthread_mutex_lock(&_mutex);
    bool isEmpty = _queue.empty();
    pthread_mutex_unlock(&_mutex);
    return isEmpty;
}
