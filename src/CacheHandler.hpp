/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CacheHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/24 23:54:58 by agaley            #+#    #+#             */
/*   Updated: 2024/06/25 01:48:34 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CACHEHANDLER_HPP
#define CACHEHANDLER_HPP

#include <map>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class CacheHandler {
 public:
  static CacheHandler& getInstance();
  static void          deleteInstance();

  void storeResponse(const HTTPRequest& request, const HTTPResponse& response);
  HTTPResponse* getResponse(const HTTPRequest& request);

 private:
  CacheHandler();
  ~CacheHandler();

  static CacheHandler*                 _instance;
  std::map<std::string, HTTPResponse*> _cache;

  std::string             _generateKey(const HTTPRequest& request) const;
  unsigned long           _hash(const std::string& str) const;
  mutable pthread_mutex_t _mutex;
};

#endif
