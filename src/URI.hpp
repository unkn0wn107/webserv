/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/03 17:01:25 by agaley            #+#    #+#             */
/*   Updated: 2024/06/18 19:30:34 by  mchenava        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENCODER_H
#define ENCODER_H

#include <iostream>
#include <map>
#include <sstream>
#include <string>

class URI {
 private:
  static char        _fromHex(const std::string& hex);
  static std::string _toHex(char c);

 public:
  struct Components {
    std::string extension;
    std::string scriptName;
    std::string pathInfo;
    std::string queryString;
  };

  static Components                         parse(const std::string& uri);
  static std::string                        encode(const std::string& uri);
  static std::string                        decode(const std::string& uri);
  static std::map<std::string, std::string> getParams(const std::string& uri);
};

#endif
