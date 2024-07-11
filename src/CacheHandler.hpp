/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CacheHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 23:54:58 by agaley            #+#    #+#             */
/*   Updated: 2024/07/08 16:28:21 by agaley           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CACHEHANDLER_HPP
#define CACHEHANDLER_HPP

#include <pthread.h>
#include <ctime>
#include <map>
#include <string>

#include "Common.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class CacheHandler {
 public:
  static const time_t MAX_AGE;

  static CacheHandler& getInstance();
  static void          deleteInstance();

  struct CacheEntry {
    HTTPResponse* response;
    time_t        timestamp;
    CacheStatus   status;

    CacheEntry();
    CacheEntry(const CacheEntry& other);
    CacheEntry& operator=(const CacheEntry& other);
    ~CacheEntry();
  };

  CacheEntry getCacheEntry(const std::string& requestString);
  void storeResponse(const HTTPRequest& request, const HTTPResponse& response);
  void deleteCache(const std::string& requestString);

 private:
  CacheHandler();
  ~CacheHandler();
  CacheHandler(const CacheHandler&);
  CacheHandler& operator=(const CacheHandler&);

  typedef std::map<std::string, CacheEntry> CacheMap;

  static CacheHandler* _instance;
  Logger&              _log;
  CacheMap             _cache;
  time_t               _maxAge;
  pthread_mutex_t      _mutex;

  std::string   _generateKey(const HTTPRequest& request) const;
  std::string   _generateKey(const std::string& requestString) const;
  unsigned long _hash(const std::string& str) const;
};

#endif
