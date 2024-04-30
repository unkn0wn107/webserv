/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileManager.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:45 by agaley            #+#    #+#             */
/*   Updated: 2024/04/30 16:11:46 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#include "FileManager.hpp"

FileManager::FileManager() {}

std::string FileManager::readFile(const std::string& path) {
  std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
  if (!file) {
    throw std::runtime_error("FileManager::readFile: Cannot open file: " +
                             path);
  }

  std::ostringstream contents;
  contents << file.rdbuf();
  file.close();
  return contents.str();
}

bool FileManager::doesFileExists(const std::string& path) {
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
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
  if (dirp == NULL) {
    throw std::runtime_error(
        "FileManager::listDirectory: Cannot open directory: " + path);
  }

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
    throw std::runtime_error(
        "FileManager::writeFile: Cannot open file for writing: " + path);
  }

  file.write(content.c_str(), content.size());
  if (!file.good()) {
    throw std::runtime_error(
        "FileManager::writeFile: Failed to write to file: " + path);
  }

  file.close();
}

void FileManager::deleteFile(const std::string& path) {
  if (remove(path.c_str()) != 0) {
    throw std::runtime_error(
        "FileManager::deleteFile: Failed to delete file: " + path);
  }
}