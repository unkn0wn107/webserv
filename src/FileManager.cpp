/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileManager.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:45 by agaley            #+#    #+#             */
/*   Updated: 2024/06/07 03:19:00 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "FileManager.hpp"
#include "Config.hpp"
#include "Logger.hpp"

FileManager::FileManager() : _log(Logger::getInstance()) {}

std::string FileManager::readFile(const std::string& path) {
  std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
  if (!file) {
    throw FileOpenException("FileManager::readFile: Cannot open file: " + path);
  }

  std::ostringstream contents;
  contents << file.rdbuf();
  if (contents.fail()) {
    throw FileReadException(
        "FileManager::readFile: Failed to read from file: " + path);
  }
  file.close();
  return contents.str();
}

// size_t FileManager::size(const std::string& path) {
//   struct stat buffer;
//   if (stat(path.c_str(), &buffer) != 0) {
//     return -1;
//   }
//   return buffer.st_size;
// }

bool FileManager::doesFileExists(const std::string& path) {
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

bool FileManager::isFileExecutable(const std::string& path) {
  struct stat buffer;
  if (stat(path.c_str(), &buffer) != 0) {
    return false;
  }
  return (buffer.st_mode & S_IXUSR) || (buffer.st_mode & S_IXGRP) ||
         (buffer.st_mode & S_IXOTH);
}

bool FileManager::isDirectory(const std::string& path) {
  struct stat buffer;
  if (stat(path.c_str(), &buffer) != 0) {
    return false;
  }
  return S_ISDIR(buffer.st_mode);
}

std::vector<std::string> FileManager::listDirectory(const std::string& path) {
  std::vector<std::string> files;
  DIR*                     dirp = opendir(path.c_str());
  if (dirp == NULL)
    throw DirectoryOpenException(
        "FileManager::listDirectory: Cannot open directory: " + path);

  struct dirent* dp;
  while ((dp = readdir(dirp)) != NULL) {
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
      files.push_back(std::string(dp->d_name));
    }
  }

  closedir(dirp);
  return files;
}

void FileManager::writeFile(const std::string& path,
                            const std::string& content) {
  std::ofstream file(path.c_str(), std::ios::out | std::ios::binary);
  if (!file) {
    throw FileOpenException(
        "FileManager::writeFile: Cannot open file for writing: " + path);
  }

  file.write(content.c_str(), content.size());
  if (!file.good()) {
    throw FileWriteException(
        "FileManager::writeFile: Failed to write to file: " + path);
  }

  file.close();
}

void FileManager::deleteFile(const std::string& path) {
  if (remove(path.c_str()) != 0) {
    throw FileDeleteException(
        "FileManager::deleteFile: Failed to delete file: " + path);
  }
}
