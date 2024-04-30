/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:13:32 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:20:35 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

template <typename T>
T Utils::stoi(const std::string& str) {
  std::istringstream iss(str);
  T                  num;
  iss >> num;
  if (iss.fail()) {
    std::cerr << "Failed to convert string to number: " << str << std::endl;
    throw std::invalid_argument("Invalid string format");
  }
  return num;
}

template <typename T>
std::string Utils::to_string(const T& num) {
  std::ostringstream oss;
  oss << num;
  return oss.str();
}

template size_t Utils::stoi<size_t>(const std::string& str);
template int    Utils::stoi<int>(const std::string& str);
template float  Utils::stoi<float>(const std::string& str);
template double Utils::stoi<double>(const std::string& str);

template std::string Utils::to_string<size_t>(const size_t& num);
template std::string Utils::to_string<int>(const int& num);
template std::string Utils::to_string<float>(const float& num);
template std::string Utils::to_string<double>(const double& num);
