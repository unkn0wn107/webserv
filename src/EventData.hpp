/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventData.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/04 19:31:05 by agaley            #+#    #+#             */
/*   Updated: 2024/07/04 20:13:56 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef EVENTDATA_HPP
#define EVENTDATA_HPP

#include "ConnectionHandler.hpp"

struct EventData {
  int                fd;
  ConnectionHandler* handler;
  pid_t              threadId;
  bool               isListening;

  EventData(int                fd,
            ConnectionHandler* ptr,
            pid_t              threadId = -1,
            bool               isListening = false);
};

#endif
