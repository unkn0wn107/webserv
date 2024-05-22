/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:13:04 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:15:53 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

class Utils {
 public:
  template <typename T>
  static T stoi(const std::string& str);

  template <typename T>
  static std::string to_string(const T& num);
  static std::string trim(const std::string& str);
};

#endif
