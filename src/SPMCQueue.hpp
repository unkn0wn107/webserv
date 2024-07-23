/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SPMCQueue.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/18 02:48:04 by agaley            #+#    #+#             */
/*   Updated: 2024/07/22 23:16:34 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef SPSCQUEUE_H
#define SPSCQUEUE_H

template <typename T>
class SPMCQueue {
 private:
  struct Node {
    Node* next;
    T     data;
    Node() : next(NULL) {}
  };

  Node*           _head;
  Node*           _tail;
  pthread_mutex_t _head_mutex;
  pthread_mutex_t _tail_mutex;

 public:
  SPMCQueue() : _head(new Node()), _tail(_head) {
    pthread_mutex_init(&_head_mutex, NULL);
    pthread_mutex_init(&_tail_mutex, NULL);
  }

  ~SPMCQueue() {
    while (Node* old_head = _head) {
      _head = _head->next;
      delete old_head;
    }
    pthread_mutex_destroy(&_head_mutex);
    pthread_mutex_destroy(&_tail_mutex);
  }

  void enqueue(const T& data) {
    Node* n = new Node();
    n->data = data;
    n->next = NULL;

    pthread_mutex_lock(&_head_mutex);
    _head->next = n;
    _head = n;
    pthread_mutex_unlock(&_head_mutex);
  }

  bool dequeue(T& result) {
    pthread_mutex_lock(&_tail_mutex);
    Node* old_tail = _tail;
    Node* new_tail = old_tail->next;

    if (new_tail == NULL) {
      pthread_mutex_unlock(&_tail_mutex);
      return false;
    }

    result = new_tail->data;
    _tail = new_tail;
    pthread_mutex_unlock(&_tail_mutex);

    delete old_tail;
    return true;
  }
};

#endif
