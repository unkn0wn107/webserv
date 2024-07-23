/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CacheHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 23:54:38 by agaley            #+#    #+#             */
/*   Updated: 2024/07/22 23:21:10 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "CacheHandler.hpp"

const time_t  CacheHandler::MAX_AGE = 3600;
CacheHandler* CacheHandler::_instance = NULL;

CacheHandler::CacheEntry::CacheEntry()
    : response(NULL), timestamp(0), status(CACHE_NOT_FOUND), waitingEventsData() {}

CacheHandler::CacheEntry::CacheEntry(const CacheEntry& other)
    : response(other.response ? new HTTPResponse(*other.response) : NULL),
      timestamp(other.timestamp),
      status(other.status),
      waitingEventsData(other.waitingEventsData) {}

CacheHandler::CacheEntry& CacheHandler::CacheEntry::operator=(const CacheEntry& other) {
  if (this != &other) {
    delete response;
    response = other.response ? new HTTPResponse(*other.response) : NULL;
    timestamp = other.timestamp;
    status = other.status;
    waitingEventsData = other.waitingEventsData;
  }
  return *this;
}

CacheHandler::CacheEntry::~CacheEntry() {
  delete response;
}

CacheHandler& CacheHandler::init(SPMCQueue<struct epoll_event>& eventQueue) {
  _instance = new CacheHandler(eventQueue);
  return *_instance;
}

CacheHandler& CacheHandler::getInstance() {
  return *_instance;
}

void CacheHandler::deleteInstance() {
  delete _instance;
  _instance = NULL;
}

CacheHandler::CacheHandler(SPMCQueue<struct epoll_event>& eventQueue)
    : _log(Logger::getInstance()), _eventQueue(eventQueue), _cache(), _maxAge(MAX_AGE) {
  pthread_mutex_init(&_mutex, NULL);
}

CacheHandler::~CacheHandler() {
  pthread_mutex_destroy(&_mutex);
}

CacheHandler::CacheEntry CacheHandler::getCacheEntry(const std::string& key,
                                                     EventData*         eventData) {
  pthread_mutex_lock(&_mutex);
  CacheMap::iterator it = _cache.find(key);

  if (it == _cache.end()) {
    _cache[key] = CacheEntry();
    _cache[key].status = CACHE_CURRENTLY_BUILDING;
    CacheEntry cacheEntry(_cache[key]);
    pthread_mutex_unlock(&_mutex);
    cacheEntry.status = CACHE_NOT_FOUND;
    return cacheEntry;
  }

  CacheEntry& entry = it->second;

  if (entry.status == CACHE_CURRENTLY_BUILDING) {
    if (eventData)
      it->second.waitingEventsData.push_back(eventData);
    pthread_mutex_unlock(&_mutex);
    return entry;
  }

  if (entry.timestamp + _maxAge <= time(NULL)) {
    if (entry.response)
      delete entry.response;
    _cache[key] = CacheEntry();
    _cache[key].status = CACHE_CURRENTLY_BUILDING;
    CacheEntry cacheEntry(_cache[key]);
    pthread_mutex_unlock(&_mutex);
    cacheEntry.status = CACHE_NOT_FOUND;
    return cacheEntry;
  }

  pthread_mutex_unlock(&_mutex);
  return entry;
}

void CacheHandler::storeResponse(const std::string& key, const HTTPResponse& response) {
  pthread_mutex_lock(&_mutex);
  CacheEntry& entry = _cache[key];
  if (entry.response)
    delete entry.response;
  entry.response = new HTTPResponse(response);
  entry.timestamp = time(NULL);
  entry.status = CACHE_FOUND;
  for (std::deque<EventData*>::iterator it = entry.waitingEventsData.begin();
       it != entry.waitingEventsData.end(); ++it) {
    struct epoll_event event;
    event.data.ptr = *it;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    _eventQueue.enqueue(event);
  }
  entry.waitingEventsData.clear();
  pthread_mutex_unlock(&_mutex);
}

void CacheHandler::deleteCache(const std::string& key) {
  pthread_mutex_lock(&_mutex);
  CacheMap::iterator it = _cache.find(key);

  if (it != _cache.end()) {
    delete it->second.response;
    _cache.erase(it);
  }

  pthread_mutex_unlock(&_mutex);
}

std::string CacheHandler::generateKey(const HTTPRequest& request) const {
  return generateKey(request.getRawRequest());
}

std::string CacheHandler::generateKey(const std::string& requestString) const {
  std::ostringstream oss;
  oss << _hash(requestString);
  return oss.str();
}

unsigned long CacheHandler::_hash(const std::string& str) const {
  // FNV-1a Hash Algorithm
  const unsigned long FNV_prime = 16777619;
  const unsigned long offset_basis = 2166136261U;
  unsigned long       hash = offset_basis;
  const char*         cstr = str.c_str();

  while (*cstr) {
    hash ^= static_cast<unsigned long>(*cstr++);
    hash *= FNV_prime;
  }

  return hash;
}
