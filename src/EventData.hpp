/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventData.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 19:31:05 by agaley            #+#    #+#             */
/*   Updated: 2024/07/23 19:19:03 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTDATA_HPP
#define EVENTDATA_HPP

#include "ConnectionHandler.hpp"

class ConnectionHandler;

struct EventData {
  int                fd;
  ConnectionHandler* handler;
  bool               isListening;
  bool               recordTime;
  time_t             startTime;
  bool               opened;

  EventData(int                fd,
            ConnectionHandler* ptr,
            bool               isListening = false,
            bool               recordTime = true,
            bool               opened = true);
  ~EventData(void);
};

#endif
