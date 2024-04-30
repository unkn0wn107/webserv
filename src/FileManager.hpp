/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileManager.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:51 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:11:52 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class FileManager {
 public:
  static bool                     doesFileExists(const std::string& path);
  static bool                     isDirectory(const std::string& path);
  static std::vector<std::string> listDirectory(const std::string& path);

  /**
   * Reads the content of a file into a string.
   * @param filePath The path to the file to be read.
   * @return The content of the file as a string.
   * @throws std::runtime_error If the file cannot be opened or read.
   */
  static std::string readFile(const std::string& filePath);

  /**
   * Writes data to a file.
   * @param filePath The path to the file where data will be written.
   * @param data The data to write to the file.
   * @throws std::runtime_error If the file cannot be opened or written to.
   */
  static void writeFile(const std::string& filePath, const std::string& data);

  void deleteFile(const std::string& path);

 private:
  FileManager();
};

#endif
