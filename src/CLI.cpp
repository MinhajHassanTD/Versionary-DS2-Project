#include "CLI.h"
#include <iostream>
#include <iomanip>
#include <opencv2/opencv.hpp>

CLI::CLI(VersionControl& versionControl)
    : versionControl_(versionControl) {

    registerCommands();
}

int CLI::run(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "No command specified." << std::endl;
        showHelp();
        return 1;
    }

    std::string commandName = args[0];

    if (commandName == "help" || commandName == "--help" || commandName == "-h") {
        showHelp();
        return 0;
    }

    auto it = commands_.find(commandName);
    if (it == commands_.end()) {
        std::cout << "Unknown command: " << commandName << std::endl;
        showHelp();
        return 1;
    }

    std::vector<std::string> commandArgs(args.begin() + 1, args.end());
    it->second.handler(commandArgs);

    return 0;
}

void CLI::showHelp() const {
    std::cout << "Versionary: An Image-Based Version Control System" << std::endl;
    std::cout << "Usage: versionary <command> [args]" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;

    for (const auto& [name, command] : commands_) {
        std::cout << "  " << std::left << std::setw(15) << name;

        // Print arguments
        std::string argsStr;
        for (const auto& arg : command.args) {
            argsStr += " <" + arg + ">";
        }
        std::cout << std::left << std::setw(25) << argsStr;

        // Print description
        std::cout << command.description << std::endl;
    }
}

void CLI::registerCommands() {
    commands_["init"] = {
        "init",
        "Initialize a new repository",
        {},
        [this](const auto& args) { handleInit(args); }
    };

    commands_["add"] = {
        "add",
        "Add an image to the staging area",
        {"image_path"},
        [this](const auto& args) { handleAdd(args); }
    };

    commands_["commit"] = {
        "commit",
        "Commit the staged image",
        {"message", "branch", "encrypt", "sign"},
        [this](const auto& args) { handleCommit(args); }
    };

    commands_["branch"] = {
        "branch",
        "List all branches",
        {},
        [this](const auto& args) { handleBranch(args); }
    };

    commands_["create-branch"] = {
        "create-branch",
        "Create a new branch",
        {"branch_name", "start_point", "description"},
        [this](const auto& args) { handleCreateBranch(args); }
    };

    commands_["switch-branch"] = {
        "switch-branch",
        "Switch to a different branch",
        {"branch_name"},
        [this](const auto& args) { handleSwitchBranch(args); }
    };

    commands_["merge"] = {
        "merge",
        "Merge a branch into the current branch",
        {"branch_name", "message"},
        [this](const auto& args) { handleMerge(args); }
    };

    commands_["delete-branch"] = {
        "delete-branch",
        "Delete a branch",
        {"branch_name"},
        [this](const auto& args) { handleDeleteBranch(args); }
    };

    commands_["verify"] = {
        "verify",
        "Verify a version's signature",
        {"version_id"},
        [this](const auto& args) { handleVerify(args); }
    };

    commands_["compare"] = {
        "compare",
        "Compare two versions of an image",
        {"version_id1", "version_id2", "output_path"},
        [this](const auto& args) { handleCompare(args); }
    };

    commands_["rollback"] = {
        "rollback",
        "Roll back to a previous version",
        {"version_id", "branch"},
        [this](const auto& args) { handleRollback(args); }
    };

    commands_["list"] = {
        "list",
        "List all versions",
        {},
        [this](const auto& args) { handleList(args); }
    };

    commands_["show"] = {
        "show",
        "Show information about a version",
        {"version_id"},
        [this](const auto& args) { handleShow(args); }
    };

    commands_["visualize"] = {
        "visualize",
        "Visualize the Quadtree structure of a version",
        {"version_id", "output_path"},
        [this](const auto& args) { handleVisualize(args); }
    };
}

void CLI::handleInit(const std::vector<std::string>& args) {
    if (versionControl_.initRepository()) {
        std::cout << "Repository initialized successfully." << std::endl;
    } else {
        std::cout << "Failed to initialize repository." << std::endl;
    }
}

void CLI::handleAdd(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: Missing image path." << std::endl;
        return;
    }

    std::string imagePath = args[0];

    if (versionControl_.addImage(imagePath)) {
        std::cout << "Image added to staging area: " << imagePath << std::endl;
    } else {
        std::cout << "Failed to add image: " << imagePath << std::endl;
    }
}

void CLI::handleCommit(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: Missing commit message." << std::endl;
        return;
    }

    std::string message = args[0];
    std::string branch = args.size() > 1 ? args[1] : "";

    // Parse encryption and signing options
    bool encrypt = false;
    bool sign = false;

    if (args.size() > 2) {
        std::string encryptArg = args[2];
        encrypt = (encryptArg == "true" || encryptArg == "1" || encryptArg == "yes");
    }

    if (args.size() > 3) {
        std::string signArg = args[3];
        sign = (signArg == "true" || signArg == "1" || signArg == "yes");
    }

    std::string versionId = versionControl_.commitImage(message, branch, encrypt, sign);

    if (!versionId.empty()) {
        std::cout << "Committed version: " << versionId;
        if (!branch.empty()) {
            std::cout << " to branch: " << branch;
        }
        if (encrypt) {
            std::cout << " (encrypted)";
        }
        if (sign) {
            std::cout << " (signed)";
        }
        std::cout << std::endl;
    } else {
        std::cout << "Failed to commit image." << std::endl;
    }
}

void CLI::handleCompare(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "Error: Missing arguments." << std::endl;
        std::cout << "Usage: versionary compare <version_id1> <version_id2> <output_path>" << std::endl;
        return;
    }

    std::string versionId1 = args[0];
    std::string versionId2 = args[1];
    std::string outputPath = args[2];

    cv::Mat diffImage = versionControl_.compareVersions(versionId1, versionId2);

    if (!diffImage.empty()) {
        if (cv::imwrite(outputPath, diffImage)) {
            std::cout << "Comparison saved to: " << outputPath << std::endl;
        } else {
            std::cout << "Failed to save comparison image." << std::endl;
        }
    } else {
        std::cout << "Failed to compare versions." << std::endl;
    }
}

void CLI::handleRollback(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: Missing version ID." << std::endl;
        return;
    }

    std::string versionId = args[0];
    std::string branch = args.size() > 1 ? args[1] : "";

    if (versionControl_.rollbackToVersion(versionId, branch)) {
        std::cout << "Rolled back to version: " << versionId;
        if (!branch.empty()) {
            std::cout << " on branch: " << branch;
        }
        std::cout << std::endl;
    } else {
        std::cout << "Failed to roll back to version: " << versionId << std::endl;
    }
}

void CLI::handleList(const std::vector<std::string>& args) {
    std::vector<VersionInfo> versions = versionControl_.getAllVersions();

    if (versions.empty()) {
        std::cout << "No versions found." << std::endl;
        return;
    }

    std::cout << "Versions:" << std::endl;
    std::cout << std::left << std::setw(40) << "ID"
              << std::setw(20) << "Timestamp"
              << std::setw(15) << "Branch"
              << "Message" << std::endl;

    std::cout << std::string(100, '-') << std::endl;

    for (const auto& version : versions) {
        std::cout << std::left << std::setw(40) << version.id
                  << std::setw(20) << version.timestamp
                  << std::setw(15) << version.branch;

        if (version.isMergeCommit) {
            std::cout << "[MERGE] ";
        }

        std::cout << version.message << std::endl;
    }

    VersionInfo currentVersion = versionControl_.getCurrentVersion();
    BranchInfo currentBranch = versionControl_.getCurrentBranch();

    if (!currentVersion.id.empty()) {
        std::cout << std::endl << "Current version: " << currentVersion.id << std::endl;
    }

    if (!currentBranch.name.empty()) {
        std::cout << "Current branch: " << currentBranch.name << std::endl;
    }
}

void CLI::handleShow(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: Missing version ID." << std::endl;
        return;
    }

    std::string versionId = args[0];

    if (!versionControl_.versionExists(versionId)) {
        std::cout << "Version not found: " << versionId << std::endl;
        return;
    }

    VersionInfo version = versionControl_.getVersion(versionId);

    std::cout << "Version Information:" << std::endl;
    std::cout << "ID:           " << version.id << std::endl;
    std::cout << "Parent ID:    " << version.parentId << std::endl;
    std::cout << "Branch:       " << version.branch << std::endl;
    std::cout << "Message:      " << version.message << std::endl;
    std::cout << "Timestamp:    " << version.timestamp << std::endl;
    std::cout << "Root Hash:    " << version.rootHash << std::endl;
    std::cout << "Image Path:   " << version.imagePath << std::endl;

    if (version.isMergeCommit) {
        std::cout << "Merge Commit: Yes" << std::endl;
        std::cout << "Merge Source: " << version.mergeSourceId << std::endl;
    } else {
        std::cout << "Merge Commit: No" << std::endl;
    }

    if (version.isEncrypted) {
        std::cout << "Encrypted:    Yes" << std::endl;
    } else {
        std::cout << "Encrypted:    No" << std::endl;
    }

    if (!version.signature.empty()) {
        std::cout << "Signed:       Yes" << std::endl;

        // Verify signature
        bool valid = versionControl_.verifyVersionSignature(version);
        std::cout << "Signature:    " << (valid ? "Valid" : "Invalid") << std::endl;
    } else {
        std::cout << "Signed:       No" << std::endl;
    }
}

void CLI::handleVisualize(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Error: Missing arguments." << std::endl;
        std::cout << "Usage: versionary visualize <version_id> <output_path>" << std::endl;
        return;
    }

    std::string versionId = args[0];
    std::string outputPath = args[1];

    if (!versionControl_.versionExists(versionId)) {
        std::cout << "Version not found: " << versionId << std::endl;
        return;
    }

    cv::Mat image = versionControl_.getVersionImage(versionId);

    if (image.empty()) {
        std::cout << "Failed to load image for version: " << versionId << std::endl;
        return;
    }

    ImageProcessor processor;
    processor.loadImage(versionControl_.getVersion(versionId).imagePath);
    cv::Mat visualized = processor.visualizeQuadtree();

    if (cv::imwrite(outputPath, visualized)) {
        std::cout << "Visualization saved to: " << outputPath << std::endl;
    } else {
        std::cout << "Failed to save visualization image." << std::endl;
    }
}

void CLI::handleBranch(const std::vector<std::string>& args) {
    std::vector<BranchInfo> branches = versionControl_.getAllBranches();

    if (branches.empty()) {
        std::cout << "No branches found." << std::endl;
        return;
    }

    BranchInfo currentBranch = versionControl_.getCurrentBranch();

    std::cout << "Branches:" << std::endl;
    std::cout << std::left << std::setw(20) << "Name"
              << std::setw(40) << "Head Version"
              << std::setw(20) << "Created"
              << "Description" << std::endl;

    std::cout << std::string(100, '-') << std::endl;

    for (const auto& branch : branches) {
        std::cout << std::left;

        // Mark current branch with an asterisk
        if (branch.name == currentBranch.name) {
            std::cout << "* " << std::setw(18) << branch.name;
        } else {
            std::cout << "  " << std::setw(18) << branch.name;
        }

        std::cout << std::setw(40) << branch.headVersionId
                  << std::setw(20) << branch.creationTimestamp
                  << branch.description << std::endl;
    }
}

void CLI::handleCreateBranch(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: Missing branch name." << std::endl;
        return;
    }

    std::string branchName = args[0];
    std::string startPoint = args.size() > 1 ? args[1] : "";
    std::string description = args.size() > 2 ? args[2] : "";

    if (versionControl_.createBranch(branchName, startPoint, description)) {
        std::cout << "Created branch: " << branchName;
        if (!startPoint.empty()) {
            std::cout << " starting at: " << startPoint;
        }
        std::cout << std::endl;
    } else {
        std::cout << "Failed to create branch: " << branchName << std::endl;
    }
}

void CLI::handleSwitchBranch(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: Missing branch name." << std::endl;
        return;
    }

    std::string branchName = args[0];

    if (versionControl_.switchBranch(branchName)) {
        std::cout << "Switched to branch: " << branchName << std::endl;
    } else {
        std::cout << "Failed to switch to branch: " << branchName << std::endl;
    }
}

void CLI::handleMerge(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: Missing branch name." << std::endl;
        return;
    }

    std::string branchName = args[0];
    std::string message = args.size() > 1 ? args[1] : "";

    std::string mergeCommitId = versionControl_.mergeBranch(branchName, message);

    if (!mergeCommitId.empty()) {
        std::cout << "Merged branch '" << branchName << "' into '" << versionControl_.getCurrentBranch().name << "'" << std::endl;
        std::cout << "Merge commit: " << mergeCommitId << std::endl;
    } else {
        std::cout << "Failed to merge branch: " << branchName << std::endl;
    }
}

void CLI::handleDeleteBranch(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: Missing branch name." << std::endl;
        return;
    }

    std::string branchName = args[0];

    if (versionControl_.deleteBranch(branchName)) {
        std::cout << "Deleted branch: " << branchName << std::endl;
    } else {
        std::cout << "Failed to delete branch: " << branchName << std::endl;
        std::cout << "Note: Cannot delete the current branch or the main branch." << std::endl;
    }
}

void CLI::handleVerify(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Error: Missing version ID." << std::endl;
        return;
    }

    std::string versionId = args[0];

    if (!versionControl_.versionExists(versionId)) {
        std::cout << "Version not found: " << versionId << std::endl;
        return;
    }

    VersionInfo version = versionControl_.getVersion(versionId);

    if (version.signature.empty()) {
        std::cout << "Version is not signed." << std::endl;
        return;
    }

    bool valid = versionControl_.verifyVersionSignature(version);

    if (valid) {
        std::cout << "Signature is valid." << std::endl;
    } else {
        std::cout << "Signature is invalid." << std::endl;
    }
}
