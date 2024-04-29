#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <map>

// Structure to hold server configuration details
struct ServerConfig
{
    std::string host;
    int port;
    std::vector<std::string> server_names;
    std::string default_error_page;
    size_t client_body_size_limit;
    std::map<std::string, std::string> default_files; // Maps directories to default files
};

// Structure to hold route specific configuration
struct RouteConfig
{
    std::vector<std::string> accepted_methods;
    std::string redirection_target;
    std::string root_directory;
    bool directory_listing_enabled;
    std::string default_file;
    std::string cgi_path;    // Path to the CGI executable
    std::string upload_path; // Directory where uploaded files will be saved
};

// Main configuration class
class Config
{
public:
    Config(const std::string &configFilePath);
    ~Config();

    // Load configuration from a file
    bool loadConfiguration();

    // Get server configuration by host and port
    ServerConfig getServerConfig(const std::string &host, int port) const;

    // Get route configuration by URL path
    RouteConfig getRouteConfig(const std::string &path) const;

private:
    std::string configFilePath;
    std::vector<ServerConfig> servers;
    std::map<std::string, RouteConfig> routes; // Maps URL paths to route configurations

    // Helper methods to parse configuration file
    void parseServerConfig(const std::string &line);
    void parseRouteConfig(const std::string &line);
};

#endif // CONFIG_H
