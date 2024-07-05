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
#include <errno.h>
#include <pthread.h>
#include <queue>
#include "Utils.hpp"

EventQueue::EventQueue() {
  pthread_mutex_init(&_mutex, NULL);
  pthread_cond_init(&_cond, NULL);
}

EventQueue::~EventQueue() {
  pthread_mutex_destroy(&_mutex);
  pthread_cond_destroy(&_cond);
}

void EventQueue::push(const struct epoll_event& event) {
  LockGuard lock(_mutex);
  _queue.push(event);
  pthread_cond_signal(&_cond);
}

bool EventQueue::pop(struct epoll_event& event) {
  LockGuard lock(_mutex);
  while (_queue.empty()) {
    pthread_cond_wait(&_cond, &_mutex);
  }
  event = _queue.front();
  _queue.pop();
  return true;
}

bool EventQueue::try_pop(struct epoll_event& event) {
  LockGuard lock(_mutex);
  if (_queue.empty()) {
    return false;
  }
  event = _queue.front();
  _queue.pop();
  return true;
}

bool EventQueue::empty() const {
  LockGuard lock(_mutex);
  return _queue.empty();
}
