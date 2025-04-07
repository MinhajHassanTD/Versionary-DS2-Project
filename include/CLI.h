#ifndef CLI_H
#define CLI_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include "VersionControl.h"

/**
 * @brief Command structure for CLI
 */
struct Command {
    std::string name;
    std::string description;
    std::vector<std::string> args;
    std::function<void(const std::vector<std::string>&)> handler;
};

/**
 * @brief CLI class for command-line interface
 *
 * This class provides a command-line interface for interacting
 * with the version control system. It includes commands for
 * initializing a repository, adding images, committing versions,
 * comparing versions, and rolling back to previous versions.
 */
class CLI {
public:
    /**
     * @brief Construct a new CLI
     *
     * @param versionControl Version control system to use
     */
    CLI(VersionControl& versionControl);

    /**
     * @brief Run the CLI
     *
     * @param args Command-line arguments
     * @return int Exit code
     */
    int run(const std::vector<std::string>& args);

    /**
     * @brief Display help information
     */
    void showHelp() const;

private:
    VersionControl& versionControl_;
    std::map<std::string, Command> commands_;

    /**
     * @brief Register all commands
     */
    void registerCommands();

    /**
     * @brief Handle init command
     *
     * @param args Command arguments
     */
    void handleInit(const std::vector<std::string>& args);

    /**
     * @brief Handle add command
     *
     * @param args Command arguments
     */
    void handleAdd(const std::vector<std::string>& args);

    /**
     * @brief Handle commit command
     *
     * @param args Command arguments
     */
    void handleCommit(const std::vector<std::string>& args);

    /**
     * @brief Handle compare command
     *
     * @param args Command arguments
     */
    void handleCompare(const std::vector<std::string>& args);

    /**
     * @brief Handle rollback command
     *
     * @param args Command arguments
     */
    void handleRollback(const std::vector<std::string>& args);

    /**
     * @brief Handle list command
     *
     * @param args Command arguments
     */
    void handleList(const std::vector<std::string>& args);

    /**
     * @brief Handle show command
     *
     * @param args Command arguments
     */
    void handleShow(const std::vector<std::string>& args);

    /**
     * @brief Handle visualize command
     *
     * @param args Command arguments
     */
    void handleVisualize(const std::vector<std::string>& args);

    /**
     * @brief Handle branch command
     *
     * @param args Command arguments
     */
    void handleBranch(const std::vector<std::string>& args);

    /**
     * @brief Handle create-branch command
     *
     * @param args Command arguments
     */
    void handleCreateBranch(const std::vector<std::string>& args);

    /**
     * @brief Handle switch-branch command
     *
     * @param args Command arguments
     */
    void handleSwitchBranch(const std::vector<std::string>& args);

    /**
     * @brief Handle merge command
     *
     * @param args Command arguments
     */
    void handleMerge(const std::vector<std::string>& args);

    /**
     * @brief Handle delete-branch command
     *
     * @param args Command arguments
     */
    void handleDeleteBranch(const std::vector<std::string>& args);

    /**
     * @brief Handle verify command
     *
     * @param args Command arguments
     */
    void handleVerify(const std::vector<std::string>& args);
};

#endif // CLI_H
