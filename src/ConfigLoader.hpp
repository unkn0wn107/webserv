#ifndef CONFIGLOADER_H
#define CONFIGLOADER_H

#include <map>
#include <string>

class ConfigLoader {
 private:
  ConfigLoader();
  ConfigLoader(const std::string* filepath);
  static ConfigLoader*               instance;
  std::map<std::string, std::string> config;
  ConfigLoader(const ConfigLoader&);
  ConfigLoader& operator=(const ConfigLoader&);

 public:
  static ConfigLoader* getInstance(const std::string* filepath);
  ~ConfigLoader();

  /**
   * Load configuration from a file.
   * @param filepath Path to the configuration file.
   * @return True if the configuration is loaded successfully, false otherwise.
   */
  bool loadConfig(const std::string& filepath);

  /**
   * Retrieve a configuration value.
   * @param key The configuration key.
   * @return The configuration value associated with the key.
   */
  std::string getConfigValue(const std::string& key) const;

  /**
   * Retrieve all config.
   * @return The configuration map.
   */
  std::map<std::string, std::string> getConfig() const;
};

#endif
