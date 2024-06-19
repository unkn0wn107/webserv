/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  mchenava < mchenava@student.42lyon.fr>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:13:04 by agaley            #+#    #+#             */
/*   Updated: 2024/06/18 19:30:34 by  mchenava        ###   ########.fr       */
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
  static std::size_t                        calculateByteLength(const std::string& input, const std::string& charset);

};

#endif
