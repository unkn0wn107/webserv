/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CacheHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 23:54:58 by agaley            #+#    #+#             */
/*   Updated: 2024/07/08 16:28:21 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CACHEHANDLER_HPP
#define CACHEHANDLER_HPP

#include <ctime>
#include <map>

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Common.hpp"

class CacheHandler {
 public:
  static const time_t MAX_AGE;

  static CacheHandler& getInstance();
  static void          deleteInstance();

  void reserveCache(std::string requestString);
  void storeResponse(const HTTPRequest& request, const HTTPResponse& response);
  HTTPResponse* getResponse(std::string requestString);
  HTTPResponse* waitResponse(std::string requestString);
  void deleteCache(std::string requestString, HTTPResponse* response);
  void deleteCache(const HTTPRequest& request);
  CacheStatus checkCache(std::string requestString);

 private:
  CacheHandler();
  ~CacheHandler();

  Logger&                                                  _log;
  static CacheHandler*                                     _instance;
  std::map<std::string, std::pair<HTTPResponse*, time_t> > _cache;
  time_t                                                   _maxAge;
  pthread_mutex_t                                          _cacheMutex;
  pthread_mutex_t                                          _mutex;

  std::string             _generateKey(const HTTPRequest& request) const;
  std::string             _generateKey(std::string requestString) const;
  unsigned long           _hash(const std::string& str) const;
};

#endif
