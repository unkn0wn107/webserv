/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CacheHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 23:54:38 by agaley            #+#    #+#             */
/*   Updated: 2024/07/08 16:28:12 by agaley           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CacheHandler.hpp"

const time_t  CacheHandler::MAX_AGE = 3600;
CacheHandler* CacheHandler::_instance = NULL;

CacheHandler::CacheEntry::CacheEntry()
    : response(NULL), timestamp(0), status(CACHE_NOT_FOUND) {}

CacheHandler::CacheEntry::CacheEntry(const CacheEntry& other)
    : response(other.response ? new HTTPResponse(*other.response) : NULL),
      timestamp(other.timestamp),
      status(other.status) {}

CacheHandler::CacheEntry& CacheHandler::CacheEntry::operator=(
    const CacheEntry& other) {
  if (this != &other) {
    delete response;
    response = other.response ? new HTTPResponse(*other.response) : NULL;
    timestamp = other.timestamp;
    status = other.status;
  }
  return *this;
}

CacheHandler::CacheEntry::~CacheEntry() {
  delete response;
}

CacheHandler& CacheHandler::getInstance() {
  if (_instance == NULL) {
    _instance = new CacheHandler();
  }
  return *_instance;
}

void CacheHandler::deleteInstance() {
  delete _instance;
  _instance = NULL;
}

CacheHandler::CacheHandler()
    : _log(Logger::getInstance()), _cache(), _maxAge(MAX_AGE) {
  pthread_mutex_init(&_mutex, NULL);
}

CacheHandler::~CacheHandler() {
  pthread_mutex_destroy(&_mutex);
}

CacheHandler::CacheEntry CacheHandler::getCacheEntry(
    const std::string& requestString) {
  std::string key = _generateKey(requestString);
  pthread_mutex_lock(&_mutex);
  CacheMap::iterator it = _cache.find(key);

  if (it == _cache.end()) {
    _log.info("CACHE_HANDLER: Cache not found, reserving");
    _cache[key] = CacheEntry();
    _cache[key].status = CACHE_CURRENTLY_BUILDING;
    CacheEntry cacheEntry(_cache[key]);
    pthread_mutex_unlock(&_mutex);
    cacheEntry.status = CACHE_NOT_FOUND;
    return cacheEntry;
  }

  CacheEntry& entry = it->second;

  if (entry.status == CACHE_CURRENTLY_BUILDING) {
    _log.warning("CACHE_HANDLER: Cache currently building");
    CacheEntry cacheEntry(entry);
    pthread_mutex_unlock(&_mutex);
    return cacheEntry;
  }

  if (entry.timestamp + _maxAge <= time(NULL)) {
    _log.warning("CACHE_HANDLER: Cache expired, deleting and reserving");
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

void CacheHandler::storeResponse(const HTTPRequest&  request,
                                 const HTTPResponse& response) {
  std::string key = _generateKey(request);
  _log.warning("CACHE_HANDLER: Storing response in cache");
  pthread_mutex_lock(&_mutex);
  CacheEntry& entry = _cache[key];
  delete entry.response;
  entry.response = new HTTPResponse(response);
  entry.timestamp = time(NULL);
  entry.status = CACHE_FOUND;
  pthread_mutex_unlock(&_mutex);
}

void CacheHandler::deleteCache(const std::string& requestString) {
  std::string key = _generateKey(requestString);
  pthread_mutex_lock(&_mutex);
  CacheMap::iterator it = _cache.find(key);

  if (it != _cache.end()) {
    _log.warning("CACHE_HANDLER: Deleting cache entry");
    delete it->second.response;
    _cache.erase(it);
  }

  pthread_mutex_unlock(&_mutex);
}

std::string CacheHandler::_generateKey(const HTTPRequest& request) const {
  return _generateKey(request.getRawRequest());
}

std::string CacheHandler::_generateKey(const std::string& requestString) const {
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
