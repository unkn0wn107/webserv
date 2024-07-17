/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:13:04 by agaley            #+#    #+#             */
/*   Updated: 2024/07/03 02:20:10 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Exception.hpp"

class Utils {
 public:
  template <typename T>
  static T stoi(const std::string& str);

  template <typename T>
  static std::string to_string(const T& num);
  static std::string trim(const std::string& str);
  static std::size_t calculateByteLength(const std::string& input,
                                         const std::string& charset);

  static void  freeCharVector(std::vector<char*>& vec);
  static char* cstr(const std::string& str);
};

#endif
