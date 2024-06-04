/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:13:32 by agaley            #+#    #+#             */
/*   Updated: 2024/06/04 14:44:16 by mchenava         ###   ########.fr       */
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

std::string Utils::trim(const std::string& str) {
  const char* whitespace = " \t\n\r\f\v";

  // Trouver le premier caractère non blanc
  size_t start = str.find_first_not_of(whitespace);
  if (start == std::string::npos)
    return "";  // La chaîne est composée uniquement d'espaces blancs

  // Trouver le dernier caractère non blanc
  size_t end = str.find_last_not_of(whitespace);

  // Extraire la sous-chaîne sans les espaces blancs au début et à la fin
  return str.substr(start, end - start + 1);
}

template size_t Utils::stoi<size_t>(const std::string& str);
template int    Utils::stoi<int>(const std::string& str);
template float  Utils::stoi<float>(const std::string& str);
template double Utils::stoi<double>(const std::string& str);

template std::string Utils::to_string<size_t>(const size_t& num);
template std::string Utils::to_string<int>(const int& num);
template std::string Utils::to_string<unsigned int>(const unsigned int& num);
template std::string Utils::to_string<float>(const float& num);
template std::string Utils::to_string<double>(const double& num);
