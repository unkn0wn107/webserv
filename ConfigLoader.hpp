#ifndef CONFIGLOADER_H
#define CONFIGLOADER_H

#include <map>
#include <string>

/**
 * Singleton class to load and manage server configuration.
 */
class ConfigLoader {
 private:
  static ConfigLoader*               instance;
  std::map<std::string, std::string> config;

  // Private constructor for singleton pattern
  ConfigLoader();

 public:
  /**
   * Destructor.
   */
  ~ConfigLoader();

  /**
   * Disallow copy and assignment to enforce singleton.
   */
  ConfigLoader(const ConfigLoader&) = delete;
  ConfigLoader& operator=(const ConfigLoader&) = delete;

  /**
   * Get the singleton instance of the ConfigLoader.
   * @return Pointer to the singleton ConfigLoader instance.
   */
  static ConfigLoader* getInstance();

  /**
   * Load configuration from a file.
   * @param filename Path to the configuration file.
   * @return True if the configuration is loaded successfully, false otherwise.
   */
  bool loadConfig(const std::string& filename);

  /**
   * Retrieve a configuration value.
   * @param key The configuration key.
   * @return The configuration value associated with the key.
   */
  std::string getConfig(const std::string& key);
};

#endif  // CONFIGLOADER_H
