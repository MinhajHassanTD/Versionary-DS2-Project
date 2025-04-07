#ifndef VERSION_CONTROL_H
#define VERSION_CONTROL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <opencv2/opencv.hpp>
#include "MerkleTree.h"
#include "ImageProcessor.h"
#include "Security.h"

/**
 * @brief Version information structure
 */
struct VersionInfo {
    std::string id;
    std::string parentId;
    std::string message;
    std::string timestamp;
    std::string rootHash;
    std::string imagePath;
    std::string branch;
    bool isMergeCommit;
    std::string mergeSourceId;
    std::string encryptionKey;
    std::string encryptionIV;
    std::vector<unsigned char> signature;
    bool isEncrypted;
};

/**
 * @brief Branch information structure
 */
struct BranchInfo {
    std::string name;
    std::string headVersionId;
    std::string description;
    std::string creationTimestamp;
};

/**
 * @brief VersionControl class for managing image versions
 *
 * This class provides functionality for tracking and managing
 * different versions of images. It includes methods for adding,
 * committing, comparing, and rolling back versions.
 */
class VersionControl {
public:
    /**
     * @brief Construct a new Version Control system
     *
     * @param repoPath Path to the repository directory
     */
    VersionControl(const std::string& repoPath);

    /**
     * @brief Initialize a new repository
     *
     * @return bool True if initialization was successful
     */
    bool initRepository();

    /**
     * @brief Add an image to the staging area
     *
     * @param imagePath Path to the image file
     * @return bool True if adding was successful
     */
    bool addImage(const std::string& imagePath);

    /**
     * @brief Commit the staged image
     *
     * @param message Commit message
     * @param branch Branch name (default: current branch)
     * @param encrypt Whether to encrypt the image (default: false)
     * @param sign Whether to sign the version (default: false)
     * @return std::string Commit ID
     */
    std::string commitImage(const std::string& message, const std::string& branch = "", bool encrypt = false, bool sign = false);

    /**
     * @brief Compare two versions of an image
     *
     * @param versionId1 First version ID
     * @param versionId2 Second version ID
     * @return cv::Mat Visualization of differences
     */
    cv::Mat compareVersions(const std::string& versionId1, const std::string& versionId2);

    /**
     * @brief Roll back to a previous version
     *
     * @param versionId Version ID to roll back to
     * @param branch Branch to update (default: current branch)
     * @return bool True if rollback was successful
     */
    bool rollbackToVersion(const std::string& versionId, const std::string& branch = "");

    /**
     * @brief Get all versions
     *
     * @return std::vector<VersionInfo> List of all versions
     */
    std::vector<VersionInfo> getAllVersions() const;

    /**
     * @brief Get a specific version
     *
     * @param versionId Version ID
     * @return VersionInfo Version information
     */
    VersionInfo getVersion(const std::string& versionId) const;

    /**
     * @brief Get the current version
     *
     * @return VersionInfo Current version information
     */
    VersionInfo getCurrentVersion() const;

    /**
     * @brief Check if a version exists
     *
     * @param versionId Version ID
     * @return bool True if the version exists
     */
    bool versionExists(const std::string& versionId) const;

    /**
     * @brief Get the image for a specific version
     *
     * @param versionId Version ID
     * @return cv::Mat Image for the version
     */
    cv::Mat getVersionImage(const std::string& versionId) const;

    /**
     * @brief Create a new branch
     *
     * @param branchName Name of the new branch
     * @param startPoint Version ID to start the branch from (default: current version)
     * @param description Description of the branch
     * @return bool True if branch creation was successful
     */
    bool createBranch(const std::string& branchName, const std::string& startPoint = "", const std::string& description = "");

    /**
     * @brief Switch to a different branch
     *
     * @param branchName Name of the branch to switch to
     * @return bool True if branch switch was successful
     */
    bool switchBranch(const std::string& branchName);

    /**
     * @brief Get all branches
     *
     * @return std::vector<BranchInfo> List of all branches
     */
    std::vector<BranchInfo> getAllBranches() const;

    /**
     * @brief Get the current branch
     *
     * @return BranchInfo Current branch information
     */
    BranchInfo getCurrentBranch() const;

    /**
     * @brief Check if a branch exists
     *
     * @param branchName Name of the branch
     * @return bool True if the branch exists
     */
    bool branchExists(const std::string& branchName) const;

    /**
     * @brief Merge a branch into the current branch
     *
     * @param branchName Name of the branch to merge
     * @param message Merge commit message
     * @return std::string Merge commit ID
     */
    std::string mergeBranch(const std::string& branchName, const std::string& message);

    /**
     * @brief Delete a branch
     *
     * @param branchName Name of the branch to delete
     * @return bool True if branch deletion was successful
     */
    bool deleteBranch(const std::string& branchName);

private:
    std::string repoPath_;
    std::string currentVersionId_;
    std::string currentBranch_;
    std::map<std::string, VersionInfo> versions_;
    std::map<std::string, BranchInfo> branches_;
    ImageProcessor imageProcessor_;
    Security security_;
    bool securityInitialized_;

    /**
     * @brief Generate a unique version ID
     *
     * @return std::string Unique ID
     */
    std::string generateVersionId() const;

    /**
     * @brief Get the current timestamp
     *
     * @return std::string Current timestamp
     */
    std::string getCurrentTimestamp() const;

    /**
     * @brief Save version information to disk
     *
     * @param versionInfo Version information to save
     * @return bool True if saving was successful
     */
    bool saveVersionInfo(const VersionInfo& versionInfo);

    /**
     * @brief Load version information from disk
     *
     * @return bool True if loading was successful
     */
    bool loadVersionInfo();

    /**
     * @brief Save the current state
     *
     * @return bool True if saving was successful
     */
    bool saveCurrentState();

    /**
     * @brief Save branch information to disk
     *
     * @param branchInfo Branch information to save
     * @return bool True if saving was successful
     */
    bool saveBranchInfo(const BranchInfo& branchInfo);

    /**
     * @brief Load branch information from disk
     *
     * @return bool True if loading was successful
     */
    bool loadBranchInfo();

    /**
     * @brief Find the common ancestor of two versions
     *
     * @param versionId1 First version ID
     * @param versionId2 Second version ID
     * @return std::string Common ancestor version ID
     */
    std::string findCommonAncestor(const std::string& versionId1, const std::string& versionId2) const;

    /**
     * @brief Merge two images
     *
     * @param baseImage Base image
     * @param ourImage Our image
     * @param theirImage Their image
     * @return cv::Mat Merged image
     */
    cv::Mat mergeImages(const cv::Mat& baseImage, const cv::Mat& ourImage, const cv::Mat& theirImage) const;

    /**
     * @brief Initialize security
     *
     * @return bool True if initialization was successful
     */
    bool initSecurity();

    /**
     * @brief Encrypt an image
     *
     * @param imagePath Path to the image
     * @param encryptionKey Encryption key
     * @param encryptionIV Encryption IV
     * @return bool True if encryption was successful
     */
    bool encryptImage(const std::string& imagePath, const std::string& encryptionKey, const std::string& encryptionIV);

    /**
     * @brief Decrypt an image
     *
     * @param imagePath Path to the encrypted image
     * @param encryptionKey Encryption key
     * @param encryptionIV Encryption IV
     * @return cv::Mat Decrypted image
     */
    cv::Mat decryptImage(const std::string& imagePath, const std::string& encryptionKey, const std::string& encryptionIV) const;

    /**
     * @brief Sign a version
     *
     * @param versionInfo Version information
     * @return std::vector<unsigned char> Signature
     */
    std::vector<unsigned char> signVersion(const VersionInfo& versionInfo) const;

    /**
     * @brief Verify a version signature
     *
     * @param versionInfo Version information
     * @return bool True if signature is valid
     */
    bool verifyVersionSignature(const VersionInfo& versionInfo) const;

    /**
     * @brief Get the path to the version directory
     *
     * @param versionId Version ID
     * @return std::string Path to the version directory
     */
    std::string getVersionPath(const std::string& versionId) const;

    /**
     * @brief Get the path to the version image
     *
     * @param versionId Version ID
     * @return std::string Path to the version image
     */
    std::string getVersionImagePath(const std::string& versionId) const;
};

#endif // VERSION_CONTROL_H
