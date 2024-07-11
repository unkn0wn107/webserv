/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CacheHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 23:54:38 by agaley            #+#    #+#             */
/*   Updated: 2024/07/08 16:28:12 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "CacheHandler.hpp"

const time_t  CacheHandler::MAX_AGE = 3600;
CacheHandler* CacheHandler::_instance = NULL;

CacheHandler& CacheHandler::getInstance() {
  if (_instance == NULL) {
    _instance = new CacheHandler();
  }
  return *_instance;
}

void CacheHandler::deleteInstance() {
  if (_instance != NULL) {
    delete _instance;
    _instance = NULL;
  }
}

CacheHandler::CacheHandler() :
_log(Logger::getInstance()),
_cache(),
_maxAge(MAX_AGE)
{
  pthread_mutex_init(&_mutex, NULL);
  pthread_mutex_init(&_cacheMutex, NULL);
}

CacheHandler::~CacheHandler() {
  for (std::map<std::string, std::pair<HTTPResponse*, time_t> >::iterator it =
           _cache.begin();
       it != _cache.end(); ++it) {
    delete it->second.first;
  }
  _cache.clear();
  pthread_mutex_destroy(&_mutex);
  pthread_mutex_destroy(&_cacheMutex);
}

CacheStatus CacheHandler::checkCache(std::string requestString) {
  std::map<std::string, std::pair<HTTPResponse*, time_t> >::iterator found;
  std::map<std::string, std::pair<HTTPResponse*, time_t> >::iterator end;
  std::string key = _generateKey(requestString);
  pthread_mutex_lock(&_mutex);
  found = _cache.find(key);
  end = _cache.end();
  if (found == end)
  {
    _log.warning("CACHE_HANDLER: Cache not found, reserving");
    this->reserveCache(requestString);
    pthread_mutex_unlock(&_mutex);
    return CACHE_NOT_FOUND;
  }
  if (found->second.first == NULL)
  {
    _log.warning("CACHE_HANDLER: Cache currently building");
    pthread_mutex_unlock(&_mutex);
    return CACHE_CURRENTLY_BUILDING;
  }
  if (found->second.second + _maxAge <= time(NULL) &&
      found->second.first != NULL) // Check cache freshness
  {
    _log.warning("CACHE_HANDLER: Cache expired, deleting");
    this->deleteCache(requestString, found->second.first);
    pthread_mutex_unlock(&_mutex);
    return CACHE_NOT_FOUND;
  }
  _log.warning("CACHE_HANDLER: Cache found");
  pthread_mutex_unlock(&_mutex);
  return CACHE_FOUND;
}

HTTPResponse* CacheHandler::getResponse(std::string requestString) {
  std::string key = _generateKey(requestString);
  HTTPResponse* response = NULL;
  _log.info("CACHE_HANDLER: Getting response");
  pthread_mutex_lock(&_mutex);
  response = new HTTPResponse(*(_cache[key].first));
  pthread_mutex_unlock(&_mutex);
  return response;
}

HTTPResponse* CacheHandler::waitResponse(std::string requestString) {
  std::string key = _generateKey(requestString);
  HTTPResponse* response = NULL;

  _log.error("CACHE_HANDLER: Waiting for response");
  pthread_mutex_lock(&_cacheMutex);
  _log.error("CACHE_HANDLER: Waiting in cache mutex");
  while (true)
  {
    usleep(10000);
    pthread_mutex_lock(&_mutex);
    response = _cache[key].first;
    pthread_mutex_unlock(&_mutex);
    if (response != NULL)
    {
      _log.error("CACHE_HANDLER: Response found");
      break;
    }
  }
  pthread_mutex_unlock(&_cacheMutex);
  return new HTTPResponse(*response);
}

void CacheHandler::reserveCache(std::string requestString) {
  std::string key = _generateKey(requestString);
  if (_cache[key].first != NULL)
    return;
  _cache[key].first = NULL;
  _cache[key].second = time(NULL);
}

void CacheHandler::storeResponse(const HTTPRequest&  request,
                                 const HTTPResponse& response) {
  std::string key = _generateKey(request);
  pthread_mutex_lock(&_mutex);
  _cache[key].first = new HTTPResponse(response);
  _cache[key].second = time(NULL);
  pthread_mutex_unlock(&_mutex);
}

void CacheHandler::deleteCache(std::string requestString,
                               HTTPResponse* response) {
  std::string key = _generateKey(requestString);
  delete response;
  _cache.erase(key);
}

void CacheHandler::deleteCache(const HTTPRequest& request) {
  std::string key = _generateKey(request);
  std::map<std::string, std::pair<HTTPResponse*, time_t> >::iterator found;
  std::map<std::string, std::pair<HTTPResponse*, time_t> >::iterator end;
  pthread_mutex_lock(&_mutex);
  found = _cache.find(key);
  end = _cache.end();
  if (found == end)
  {
    pthread_mutex_unlock(&_mutex);
    return;
  }
  delete found->second.first;
  _cache.erase(found);
  pthread_mutex_unlock(&_mutex);
}

std::string CacheHandler::_generateKey(const HTTPRequest& request) const {
  std::ostringstream oss;
  oss << _hash(request.getRawRequest());
  return oss.str();
}

std::string CacheHandler::_generateKey(std::string requestString) const {
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
