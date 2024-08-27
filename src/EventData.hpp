/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventData.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 19:31:05 by agaley            #+#    #+#             */
/*   Updated: 2024/07/05 21:33:12 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTDATA_HPP
#define EVENTDATA_HPP

#include <pthread.h>

#include "ConnectionHandler.hpp"

class ConnectionHandler;

struct EventData {
  int                fd;
  ConnectionHandler* handler;
  pid_t              threadId;
  bool               isListening;
  bool               recordTime;
  time_t             startTime;
  bool               opened;
  pthread_mutex_t    mutex;

  int                getFd();
  ConnectionHandler* getHandler();
  pid_t              getThreadId();
  bool               getIsListening();
  bool               getRecordTime();
  time_t             getStartTime();
  bool               getOpened();

  void               setFd(int fd);
  void               setHandler(ConnectionHandler* handler);
  void               setThreadId(pid_t pid);
  void               setIsListening(bool isListening);
  void               setRecordTime(bool record);
  void               setStartTime(time_t time);
  void               setOpened(bool opened);

  EventData(int                fd,
            ConnectionHandler* ptr,
            pid_t              threadId = -1,
            bool               isListening = false,
            bool               recordTime = true,
            bool               opened = true);
  ~EventData(void);
};

#endif
