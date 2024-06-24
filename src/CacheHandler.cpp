/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CacheHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 23:54:38 by agaley            #+#    #+#             */
/*   Updated: 2024/06/25 01:48:57 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "CacheHandler.hpp"

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

CacheHandler::CacheHandler() {
  pthread_mutex_init(&_mutex, NULL);
}

CacheHandler::~CacheHandler() {
  for (std::map<std::string, HTTPResponse*>::iterator it = _cache.begin();
       it != _cache.end(); ++it) {
    delete it->second;
  }
  _cache.clear();
  pthread_mutex_destroy(&_mutex);
}

HTTPResponse* CacheHandler::getResponse(const HTTPRequest& request) {
  std::string key = _generateKey(request);

  pthread_mutex_lock(&_mutex);
  std::map<std::string, HTTPResponse*>::const_iterator it = _cache.find(key);
  if (it != _cache.end()) {
    HTTPResponse* response = new HTTPResponse(*(it->second));
    pthread_mutex_unlock(&_mutex);
    return response;
  } else {
    pthread_mutex_unlock(&_mutex);
    return NULL;
  }
}

void CacheHandler::storeResponse(const HTTPRequest&  request,
                                 const HTTPResponse& response) {
  std::string key = _generateKey(request);
  pthread_mutex_lock(&_mutex);
  if (_cache.find(key) == _cache.end())
    _cache[key] = new HTTPResponse(response);
  pthread_mutex_unlock(&_mutex);
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
