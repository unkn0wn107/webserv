/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileManager.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mchenava <mchenava@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:51 by agaley            #+#    #+#             */
/*   Updated: 2024/06/14 14:15:22 by mchenava         ###   ########.fr       */
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

#include "Exception.hpp"
#include "Logger.hpp"

class FileManager {
 public:
  static bool                     doesFileExists(const std::string& path);
  static bool                     isFileExecutable(const std::string& path);
  static bool                     isDirectory(const std::string& path);
  static std::vector<std::string> listDirectory(const std::string& path);

  /**
   * Reads the content of a file into a string.
   * @param filePath The path to the file to be read.
   * @return The content of the file as a string.
   * @throws std::runtime_error If the file cannot be opened or read.
   */
  static std::string readFile(const std::string& filePath);
  static int         getFileSize(const std::string& filePath);
  /**
   * Writes data to a file.
   * @param filePath The path to the file where data will be written.
   * @param data The data to write to the file.
   * @throws std::runtime_error If the file cannot be opened or written to.
   */
  static void writeFile(const std::string& filePath, const std::string& data);
  static void deleteFile(const std::string& path);

  class FileOpenException : public Exception {
   public:
    FileOpenException(const std::string& message) : Exception(message) {}
  };

  class FileReadException : public Exception {
   public:
    FileReadException(const std::string& message) : Exception(message) {}
  };

  class FileWriteException : public Exception {
   public:
    FileWriteException(const std::string& message) : Exception(message) {}
  };

  class FileDeleteException : public Exception {
   public:
    FileDeleteException(const std::string& message) : Exception(message) {}
  };

  class DirectoryOpenException : public Exception {
   public:
    DirectoryOpenException(const std::string& message) : Exception(message) {}
  };

 private:
  FileManager();
  Logger& _log;
};

#endif
