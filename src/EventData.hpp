/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventData.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 19:31:05 by agaley            #+#    #+#             */
/*   Updated: 2024/07/23 02:10:14 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTDATA_HPP
#define EVENTDATA_HPP

#include "ConnectionHandler.hpp"

class ConnectionHandler;

struct EventData {
  int                fd;
  ConnectionHandler* handler;
  pthread_mutex_t&   requestTimesMutex;
  pid_t              threadId;
  bool               isListening;
  bool               recordTime;
  time_t             startTime;
  bool               opened;

  EventData(int                fd,
            ConnectionHandler* ptr,
            pthread_mutex_t&   requestTimesMutex,
            pid_t              threadId = -1,
            bool               isListening = false,
            bool               recordTime = true,
            bool               opened = true);
};

#endif
