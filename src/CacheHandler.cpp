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

CacheHandler::CacheHandler() : _maxAge(MAX_AGE) {
  pthread_mutex_init(&_mutex, NULL);
}

CacheHandler::~CacheHandler() {
  for (std::map<std::string, std::pair<HTTPResponse*, time_t> >::iterator it =
           _cache.begin();
       it != _cache.end(); ++it) {
    delete it->second.first;
  }
  _cache.clear();
  pthread_mutex_destroy(&_mutex);
}

int CacheHandler::getResponse(const HTTPRequest& request,
                              HTTPResponse&      response) {
  LockGuard lock(_mutex);
  std::map<std::string, std::pair<HTTPResponse*, time_t> >::const_iterator it =
      _cache.find(_generateKey(request));
  if (it != _cache.end()) {
    if (it->second.first == NULL) {  // Check if cache is currently building
      return -1;
    } else if (it->second.second + _maxAge >
               time(NULL)) {  // Check cache freshness
      response = *(it->second.first);
      return 1;
    } else {
      delete it->second.first;  // Response
      _cache.erase(it->first);
    }
  }
  return 0;
}

void CacheHandler::reserveCache(const HTTPRequest&  request) {
  std::string key = _generateKey(request);
  LockGuard   lock(_mutex);
  if (_cache.find(key) == _cache.end())
    _cache[key] = std::pair<HTTPResponse*, time_t>(NULL, time(NULL));
}

void CacheHandler::storeResponse(const HTTPRequest&  request,
                                 const HTTPResponse& response) {
  std::string key = _generateKey(request);
  LockGuard   lock(_mutex);
  if (_cache[key].first == NULL)
    _cache.erase(key);
  if (_cache.find(key) == _cache.end())
    _cache[key] = std::make_pair(new HTTPResponse(response), time(NULL));
}

void CacheHandler::deleteCache(const HTTPRequest&  request) {
  std::string key = _generateKey(request);
  LockGuard   lock(_mutex);
  if (_cache.find(key) != _cache.end())
    _cache.erase(key);
}

std::string CacheHandler::_generateKey(const HTTPRequest& request) const {
  std::ostringstream oss;
  oss << _hash(request.getRawRequest());
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
