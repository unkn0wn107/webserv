/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: agaley <agaley@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/30 16:11:17 by agaley            #+#    #+#             */
/*   Updated: 2024/05/04 01:49:20 by agaley           ###   ########lyon.fr   */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGLOADER_H
#define CONFIGLOADER_H

#include <map>
#include <string>

class ConfigLoader {
 private:
  ConfigLoader();
  static ConfigLoader*               instance;
  std::map<std::string, std::string> config;

  // Disallow the copy constructor and copy assignment operator
  ConfigLoader(const ConfigLoader&);
  ConfigLoader& operator=(const ConfigLoader&);

 public:
  static ConfigLoader& getInstance();
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

  static const std::string DEFAULT_HOST;
  static const std::string DEFAULT_PORT;
  static const std::string DEFAULT_ROOT;
  static const std::string DEFAULT_MAX_CLIENT_BODY_SIZE;
  static const std::string DEFAULT_FILE_NAME;
};

#endif
