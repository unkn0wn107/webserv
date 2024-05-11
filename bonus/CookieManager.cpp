#include "CookieManager.hpp"

CookieManager::CookieManager() {
  // Constructor implementation
}

std::string CookieManager::createCookie(const std::string& name,
                                        const std::string& value) {
  // Create a simple cookie string
  std::string cookie = name + "=" + value + "; Path=/; HttpOnly";
  cookies[name] = value;
  return cookie;
}

std::string CookieManager::getCookie(const std::string& name) {
  // Retrieve a cookie value by name
  std::map<std::string, std::string>::iterator it = cookies.find(name);
  if (it != cookies.end()) {
    return it->second;
  }
  return "";  // Return empty string if cookie not found
}

void CookieManager::deleteCookie(const std::string& name) {
  // Delete a cookie by setting its expiry to a past date
  std::map<std::string, std::string>::iterator it = cookies.find(name);
  if (it != cookies.end()) {
    cookies.erase(it);
  }
}
