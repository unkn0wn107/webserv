/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CacheHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 23:54:58 by agaley            #+#    #+#             */
/*   Updated: 2024/07/23 15:44:58 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CACHEHANDLER_HPP
#define CACHEHANDLER_HPP

#include <pthread.h>
#include <ctime>
#include <deque>
#include <map>
#include <string>

#include "Common.hpp"
#include "EventData.hpp"
#include "EventQueue.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

struct EventData;

class CacheHandler {
 public:
  static const time_t MAX_AGE;

  static CacheHandler& init(EventQueue& eventQueue);
  static CacheHandler& getInstance();
  static void          deleteInstance();

  struct CacheEntry {
    HTTPResponse*          response;
    time_t                 timestamp;
    CacheStatus            status;
    std::deque<EventData*> waitingEventsData;

    CacheEntry();
    CacheEntry(const CacheEntry& other);
    CacheEntry& operator=(const CacheEntry& other);
    ~CacheEntry();
  };

  std::string generateKey(const HTTPRequest& request) const;
  std::string generateKey(const std::string& requestString) const;
  CacheStatus getCacheStatus(const std::string& key, EventData* eventData);
  HTTPResponse *getResponse(const std::string& key);
  void        storeResponse(const std::string& key, const HTTPResponse& response);
  void        deleteCache(const std::string& key);
  static      pthread_mutex_t mutex();

 private:
  CacheHandler(EventQueue& eventQueue);
  ~CacheHandler();
  CacheHandler(const CacheHandler&);
  CacheHandler& operator=(const CacheHandler&);

  typedef std::map<std::string, CacheEntry> CacheMap;

  static CacheHandler* _instance;
  Logger&              _log;
  EventQueue&          _eventQueue;
  CacheMap             _cache;
  time_t               _maxAge;
  pthread_mutex_t      _mutex;

  unsigned long _hash(const std::string& str) const;
};

#endif
