#ifndef COOKIE_MANAGER_H
#define COOKIE_MANAGER_H

#include <map>
#include <string>

class CookieManager {
 public:
  /**
   * @brief Create a cookie with a specified name and value.
   *
   * @param name The name of the cookie.
   * @param value The value of the cookie.
   * @return std::string The formatted cookie string.
   */
  std::string createCookie(const std::string& name, const std::string& value);

  /**
   * @brief Retrieve the value of a cookie by its name.
   *
   * @param name The name of the cookie.
   * @return std::string The value of the cookie, or an empty string if not
   * found.
   */
  std::string getCookie(const std::string& name);

  /**
   * @brief Delete a cookie by its name.
   *
   * @param name The name of the cookie to delete.
   */
  void deleteCookie(const std::string& name);

 private:
  std::map<std::string, std::string>
      cookies;  // Map to store cookies by their names.
};

#endif
