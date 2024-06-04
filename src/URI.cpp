/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 17:01:22 by agaley            #+#    #+#             */
/*   Updated: 2024/06/03 18:26:31 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "URI.hpp"

std::string URI::encode(const std::string& uri) {
  std::string encodedURI = "";
  for (size_t i = 0; i < uri.length(); i++) {
    if (isalnum(uri[i]) || uri[i] == '-' || uri[i] == '_' || uri[i] == '.' ||
        uri[i] == '~' || uri[i] == '?' || uri[i] == '&' || uri[i] == '=') {
      encodedURI += uri[i];
    } else {
      encodedURI += '%';
      encodedURI += _toHex(uri[i]);
    }
  }
  return encodedURI;
}

std::string URI::decode(const std::string& uri) {
  std::string decodedURI = "";
  for (size_t i = 0; i < uri.length(); i++) {
    if (uri[i] == '%' && (i + 2) < uri.length()) {
      decodedURI += _fromHex(uri.substr(i + 1, 2));
      i += 2;
    } else
      decodedURI += uri[i];
  }
  return decodedURI;
}

std::string URI::_toHex(char c) {
  std::stringstream ss;
  ss << std::hex << (int)c;
  return ss.str();
}

char URI::_fromHex(const std::string& hex) {
  int                value = 0;
  std::istringstream ss(hex);
  ss >> std::hex >> value;
  return (char)value;
}

std::map<std::string, std::string> URI::getParams(const std::string& uri) {
  std::map<std::string, std::string> queryParams;
  size_t                             queryStart = uri.find('?');
  if (queryStart != std::string::npos) {
    std::string        queryString = uri.substr(queryStart + 1);
    std::istringstream iss(queryString);
    std::string        param;
    while (std::getline(iss, param, '&')) {
      size_t separatorPos = param.find('=');
      if (separatorPos != std::string::npos) {
        std::string key = param.substr(0, separatorPos);
        std::string value = param.substr(separatorPos + 1);
        queryParams[key] = value;
      }
    }
  }
  return queryParams;
}