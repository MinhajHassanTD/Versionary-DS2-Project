#include "cli/VersionaryCLI.h"
#include "gui/MainWindow.h"
#include <iostream>
#include <string>
#include <QApplication>

int main(int argc, char* argv[]) {
    // Check if we should use GUI or CLI
    bool useGui = true;
    
    // If arguments provided, use CLI mode
    if (argc > 1) {
        useGui = false;
    }

    if (useGui) {
        // Start GUI application
        QApplication app(argc, argv);
        MainWindow mainWindow;
        mainWindow.show();
        return app.exec();
    } else {
        // CLI mode
        VersionaryCLI cli;
        
        if (argc > 1) {
            // Command line mode with arguments
            std::string commandLine;
            for (int i = 1; i < argc; i++) {
                commandLine += std::string(argv[i]) + " ";
            }
            
            cli.executeCommand(commandLine);
        } else {
            // Interactive CLI mode
            std::cout << "Versionary - Image Version Control System" << std::endl;
            std::cout << "Type 'help' for usage information or 'exit' to quit." << std::endl;
            
            std::string commandLine;
            while (true) {
                std::cout << "> ";
                std::getline(std::cin, commandLine);
                
                if (commandLine == "exit") {
                    break;
                }
                
                cli.executeCommand(commandLine);
            }
        }
    }
    
    return 0;
}