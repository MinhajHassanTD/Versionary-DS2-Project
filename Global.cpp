#include "Global.h"
#include <fstream>
#include <iostream>
#include <sstream>

// Define shared variables
std::map<int, std::string> versionRepository;
int currentVersion = 0;

// Save the version repository to a file
void saveVersionRepository(const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return;
    }
    
    // Save each version and its hash
    for (const auto& pair : versionRepository) {
        outfile << pair.first << " " << pair.second << std::endl;
    }
    
    std::cout << "Version repository saved to " << filename << std::endl;
}

// Load the version repository from a file
bool loadVersionRepository(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cout << "No previous version repository found." << std::endl;
        return false;
    }
    
    // Clear existing repository
    versionRepository.clear();
    currentVersion = 0;
    
    // Read each version and its hash
    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        int version;
        std::string hash;
        
        if (!(iss >> version >> hash)) {
            std::cerr << "Error parsing line: " << line << std::endl;
            continue;
        }
        
        versionRepository[version] = hash;
        
        // Update currentVersion to be the highest version number
        if (version > currentVersion) {
            currentVersion = version;
        }
    }
    
    std::cout << "Loaded " << versionRepository.size() << " versions from repository" << std::endl;
    return true;
}
