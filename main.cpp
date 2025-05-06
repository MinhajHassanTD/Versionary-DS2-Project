#include "CLI.h"
#include "Global.h"
#include <iostream>
#include <csignal>

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived interrupt signal. Saving repository and exiting...\n";
    saveVersionRepository();
    exit(0);
}

int main() {
    std::cout << "Welcome to Versionary - Image-Based Version Control System\n";
    
    try {
        // Set up signal handling for graceful shutdown
        std::signal(SIGINT, signalHandler);
        
        // Load any existing version repository
        loadVersionRepository("repository.dat");
        
        // Initialize CLI
        CLI cli;
        cli.run();
        
        // Save repository before exiting
        saveVersionRepository();
    } 
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
