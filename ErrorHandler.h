#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <string>
#include <exception>

/**
 * ErrorHandler class is responsible for handling exceptions and errors
 * that occur during the server's operation. It provides a centralized
 * mechanism to manage error logging and response generation for errors.
 */
class ErrorHandler {
public:
    /**
     * Default constructor for ErrorHandler.
     */
    ErrorHandler();

    /**
     * Handles exceptions and errors by logging them and optionally
     * generating an appropriate response.
     * @param e The exception object caught.
     */
    void handleError(const std::exception& e);

    /**
     * Handles generic error messages.
     * @param errorMessage The error message to be logged.
     */
    void handleError(const std::string& errorMessage);

private:
    /**
     * Logs the error details to a log file or standard error.
     * @param message The error message to log.
     */
    void logError(const std::string& message);
};

#endif // ERRORHANDLER_H
