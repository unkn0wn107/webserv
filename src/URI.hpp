/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 17:01:25 by agaley            #+#    #+#             */
/*   Updated: 2024/06/25 23:53:10 by agaley           ###   ########.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENCODER_H
#define ENCODER_H

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

class URI {
 private:
  static char        _fromHex(const std::string& hex);
  static std::string _toHex(char c);

  static std::string _parsePath(std::string::const_iterator& it,
                                std::string::const_iterator  end);
  static std::string _parseQuery(std::string::const_iterator& it,
                                 std::string::const_iterator  end);

  static std::string _getExtension(const std::string& path);
  static std::string _getScriptName(const std::string& path);
  static std::string _getPathInfo(const std::string& path);

 public:
  struct Components {
    // Normalized
    std::string path;
    std::string query;

    // Legacy
    std::string extension;
    std::string scriptName;
    std::string pathInfo;
  };

  static Components                         parse(const std::string& uri);
  static std::string                        encode(const std::string& uri);
  static std::string                        decode(const std::string& uri);
  static std::map<std::string, std::string> getParams(const std::string& uri);
};

#endif
