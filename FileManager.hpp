#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <fstream>
#include <sstream>
#include <string>

class FileManager {
 public:
  FileManager() {}

  /**
   * Reads the content of a file into a string.
   * @param filePath The path to the file to be read.
   * @return The content of the file as a string.
   * @throws std::runtime_error If the file cannot be opened or read.
   */
  std::string readFile(const std::string& filePath) {
    std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
      throw std::runtime_error("FileManager: Unable to open file: " + filePath);
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    file.close();
    return contents.str();
  }

  /**
   * Writes data to a file.
   * @param filePath The path to the file where data will be written.
   * @param data The data to write to the file.
   * @throws std::runtime_error If the file cannot be opened or written to.
   */
  void writeFile(const std::string& filePath, const std::string& data) {
    std::ofstream file(filePath.c_str(), std::ios::out | std::ios::binary);
    if (!file) {
      throw std::runtime_error(
          "FileManager: Unable to open file for writing: " + filePath);
    }

    file.write(data.c_str(), data.size());
    if (!file.good()) {
      throw std::runtime_error(
          "FileManager: Error occurred while writing to file: " + filePath);
    }

    file.close();
  }
};

#endif  // FILEMANAGER_H
