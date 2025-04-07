#include <iostream>
#include <string>
#include <vector>
#include <QApplication>
#include "VersionControl.h"
#include "CLI.h"
#include "GUI.h"

void printUsage() {
    std::cout << "Versionary: An Image-Based Version Control System" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  versionary [--cli] [command] [args]" << std::endl;
    std::cout << "  versionary --gui" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --cli       Run in command-line mode (default)" << std::endl;
    std::cout << "  --gui       Run in graphical mode" << std::endl;
    std::cout << "  --help      Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  init                Initialize a new repository" << std::endl;
    std::cout << "  add <image_path>    Add an image to the staging area" << std::endl;
    std::cout << "  commit <message> [branch] [encrypt] [sign]" << std::endl;
    std::cout << "                      Commit the staged image" << std::endl;
    std::cout << "  compare <v1> <v2> <output_path>" << std::endl;
    std::cout << "                      Compare two versions and save the result" << std::endl;
    std::cout << "  rollback <version_id> [branch]" << std::endl;
    std::cout << "                      Roll back to a previous version" << std::endl;
    std::cout << "  list                List all versions" << std::endl;
    std::cout << "  show <version_id>   Show version information" << std::endl;
    std::cout << "  visualize <version_id> <output_path>" << std::endl;
    std::cout << "                      Visualize the Quadtree structure" << std::endl;
    std::cout << "  branch              List all branches" << std::endl;
    std::cout << "  create-branch <name> [start_point] [description]" << std::endl;
    std::cout << "                      Create a new branch" << std::endl;
    std::cout << "  switch-branch <name>" << std::endl;
    std::cout << "                      Switch to a different branch" << std::endl;
    std::cout << "  merge <branch_name> [message]" << std::endl;
    std::cout << "                      Merge a branch into the current branch" << std::endl;
    std::cout << "  delete-branch <name>" << std::endl;
    std::cout << "                      Delete a branch" << std::endl;
    std::cout << "  verify <version_id> Verify a version's signature" << std::endl;
}

int main(int argc, char* argv[]) {
    // Default repository path
    std::string repoPath = ".versionary";

    // Parse command-line arguments
    std::vector<std::string> args;
    bool useGUI = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--gui") {
            useGUI = true;
        } else if (arg == "--cli") {
            useGUI = false;
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            args.push_back(arg);
        }
    }

    // Create version control system
    VersionControl versionControl(repoPath);

    // Run in GUI mode
    if (useGUI) {
        QApplication app(argc, argv);

        GUI gui(versionControl);
        gui.show();

        return app.exec();
    }

    // Run in CLI mode
    CLI cli(versionControl);
    return cli.run(args);
}
