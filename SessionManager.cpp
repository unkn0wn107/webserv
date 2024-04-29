#include "SessionManager.hpp"
#include <map>
#include <string>

// Singleton instance
SessionManager* SessionManager::instance = nullptr;

SessionManager* SessionManager::getInstance() {
  if (instance == nullptr) {
    instance = new SessionManager();
  }
  return instance;
}

SessionManager::SessionManager() {
  // Constructor for SessionManager
}

SessionManager::~SessionManager() {
  // Destructor for SessionManager
  // Clean up any allocated resources if necessary
}

std::string SessionManager::createSession() {
  // Generate a unique session ID
  static int  sessionCounter = 0;
  std::string sessionId = "sess_" + std::to_string(sessionCounter++);
  sessions[sessionId] = std::map<std::string, std::string>();
  return sessionId;
}

std::map<std::string, std::string> SessionManager::getSession(
    const std::string& sessionId) {
  // Retrieve session data by session ID
  if (sessions.find(sessionId) != sessions.end()) {
    return sessions[sessionId];
  } else {
    return std::map<std::string,
                    std::string>();  // Return empty map if session not found
  }
}

void SessionManager::setSession(
    const std::string&                        sessionId,
    const std::map<std::string, std::string>& sessionData) {
  // Set session data for a given session ID
  sessions[sessionId] = sessionData;
}

void SessionManager::deleteSession(const std::string& sessionId) {
  // Delete a session by its ID
  sessions.erase(sessionId);
}
