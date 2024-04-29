#include "server.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string configFilePath = "default_config.cfg"; // Default configuration file path

    // Check if a configuration file path is provided as an argument
    if (argc > 1) {
        configFilePath = argv[1];
    }

    try {
        // Create and initialize the server with the configuration file
        Server server(configFilePath);
        server.initialize();

        // Run the server
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
