#ifndef CGI_HANDLER_H
#define CGI_HANDLER_H

#include <string>
#include <map>
#include "http_request.h"
#include "http_response.h"

// Class to handle CGI scripts execution
class CGIHandler
{
public:
    CGIHandler();
    ~CGIHandler();

    // Execute the CGI script and generate the HTTP response
    bool executeCGI(const HttpRequest &request, HttpResponse &response, const std::string &scriptPath);

private:
    // Helper method to set up the environment for the CGI script
    void setupEnvironment(const HttpRequest &request);

    // Helper method to parse the output from the CGI script
    void parseCGIOutput(const std::string &output, HttpResponse &response);

    // Environment variables for CGI execution
    std::map<std::string, std::string> envVariables;

    // Helper method to convert headers to CGI environment variables
    void headersToEnv(const std::map<std::string, std::string> &headers);

    // Helper method to handle POST data for CGI
    void handlePostData(const HttpRequest &request);

    // Helper method to create the CGI process and handle its execution
    bool createAndRunCGIProcess(const std::string &scriptPath, const std::string &requestData, std::string &outputData);
};

#endif // CGI_HANDLER_H
