/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:13:32 by agaley            #+#    #+#             */
/*   Updated: 2024/06/20 12:17:58 by mchenava         ###   ########.fr       */
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

size_t Utils::calculateByteLength(const std::string& input,
                                  const std::string& charset) {
  if (charset == "UTF-8") {
    std::size_t length = 0;
    for (std::size_t i = 0; i < input.length(); ++i) {
      unsigned char c = static_cast<unsigned char>(input[i]);
      if ((c & 0x80) == 0) {  // 0xxxxxxx, 1 byte
        length += 1;
      } else if ((c & 0xE0) == 0xC0) {  // 110xxxxx, 2 bytes
        length += 2;
        ++i;                            // Skip next byte
      } else if ((c & 0xF0) == 0xE0) {  // 1110xxxx, 3 bytes
        length += 3;
        i += 2;                         // Skip next two bytes
      } else if ((c & 0xF8) == 0xF0) {  // 11110xxx, 4 bytes
        length += 4;
        i += 3;  // Skip next three bytes
      }
    }
    return length;
  } else if (charset == "ISO-8859-1" || charset == "ASCII") {
    return input.size();  // These encodings are usually 1 byte per character
  }
  // Add other encodings if necessary
  return input
      .size();  // Returns the default length if the encoding is not handled
}

template size_t       Utils::stoi<size_t>(const std::string& str);
template int          Utils::stoi<int>(const std::string& str);
template unsigned int Utils::stoi<unsigned int>(const std::string& str);
template float        Utils::stoi<float>(const std::string& str);
template double       Utils::stoi<double>(const std::string& str);

template std::string Utils::to_string<size_t>(const size_t& num);
template std::string Utils::to_string<int>(const int& num);
template std::string Utils::to_string<unsigned int>(const unsigned int& num);
template std::string Utils::to_string<float>(const float& num);
template std::string Utils::to_string<double>(const double& num);
template std::string Utils::to_string<ssize_t>(const ssize_t& num);

void Utils::freeCharVector(std::vector<char*>& vec) {
  for (size_t i = 0; i < vec.size(); i++)
    delete[] vec[i];
  vec.clear();
}

char* Utils::cstr(const std::string& str) {
  char* result = new char[str.length() + 1];
  if (result == NULL)
    throw Exception("Failed to allocate memory");
  std::strcpy(result, str.c_str());
  return result;
}
