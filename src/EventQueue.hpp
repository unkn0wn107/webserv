/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventQueue.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/27 16:00:17 by  mchenava         #+#    #+#             */
/*   Updated: 2024/06/27 16:02:40 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTQUEUE_HPP
#define EVENTQUEUE_HPP

#include <queue>
#include <pthread.h>
#include <sys/epoll.h>

class EventQueue {
public:
    EventQueue();
    ~EventQueue();
    void push(const struct epoll_event& event);
    bool pop(struct epoll_event& event);
    bool try_pop(struct epoll_event& event);
    bool empty() const;

private:
    std::queue<struct epoll_event> _queue;
    mutable pthread_mutex_t _mutex;
    pthread_cond_t _cond;
};

#endif
