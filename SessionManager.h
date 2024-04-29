#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <map>
#include <string>

/**
 * SessionManager class is responsible for managing user sessions.
 * It provides functionalities to create, retrieve, update, and delete sessions.
 * Each session is identified by a unique session ID and contains a map of key-value pairs.
 */
class SessionManager
{
public:
	/**
	 * Creates a new session and returns the session ID.
	 * @return std::string The unique session ID.
	 */
	std::string createSession();

	/**
	 * Retrieves the session data for a given session ID.
	 * @param sessionId The session ID.
	 * @return std::map<std::string, std::string> The session data as a map of key-value pairs.
	 */
	std::map<std::string, std::string> getSession(const std::string &sessionId);

	/**
	 * Updates the session data for a given session ID.
	 * @param sessionId The session ID.
	 * @param sessionData The session data as a map of key-value pairs.
	 */
	void setSession(const std::string &sessionId, const std::map<std::string, std::string> &sessionData);

	/**
	 * Deletes a session by its ID.
	 * @param sessionId The session ID to be deleted.
	 */
	void deleteSession(const std::string &sessionId);

private:
	// Map to store session data, where the key is the session ID and the value is the session data.
	std::map<std::string, std::map<std::string, std::string>> sessions;

	// Generates a unique session ID.
	std::string generateSessionId();
};

#endif // SESSION_MANAGER_H
