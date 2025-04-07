#include "VersionControl.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <random>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

VersionControl::VersionControl(const std::string& repoPath)
    : repoPath_(repoPath), currentVersionId_(""), currentBranch_("main"), securityInitialized_(false) {

    // Create repository directory if it doesn't exist
    if (!fs::exists(repoPath_)) {
        fs::create_directories(repoPath_);
    }

    // Load existing version information
    loadVersionInfo();

    // Load branch information
    loadBranchInfo();

    // Initialize security
    initSecurity();
}

bool VersionControl::initRepository() {
    // Create necessary directories
    fs::create_directories(repoPath_ + "/versions");
    fs::create_directories(repoPath_ + "/staging");
    fs::create_directories(repoPath_ + "/branches");
    fs::create_directories(repoPath_ + "/security");

    // Create initial state file
    std::ofstream stateFile(repoPath_ + "/state.json");
    if (!stateFile.is_open()) {
        return false;
    }

    json state;
    state["current_version"] = "";
    state["current_branch"] = "main";
    stateFile << state.dump(4);
    stateFile.close();

    // Create main branch
    BranchInfo mainBranch;
    mainBranch.name = "main";
    mainBranch.headVersionId = "";
    mainBranch.description = "Main branch";
    mainBranch.creationTimestamp = getCurrentTimestamp();

    if (!saveBranchInfo(mainBranch)) {
        return false;
    }

    // Add to branches map
    branches_["main"] = mainBranch;
    currentBranch_ = "main";

    return true;
}

bool VersionControl::addImage(const std::string& imagePath) {
    // Check if file exists
    if (!fs::exists(imagePath)) {
        return false;
    }

    // Copy image to staging area
    std::string stagingPath = repoPath_ + "/staging/image.png";

    try {
        fs::copy_file(imagePath, stagingPath, fs::copy_options::overwrite_existing);
    } catch (const fs::filesystem_error& e) {
        return false;
    }

    return true;
}

std::string VersionControl::commitImage(const std::string& message, const std::string& branch, bool encrypt, bool sign) {
    // Check if there's an image in the staging area
    std::string stagingPath = repoPath_ + "/staging/image.png";
    if (!fs::exists(stagingPath)) {
        return "";
    }

    // Determine which branch to commit to
    std::string targetBranch = branch.empty() ? currentBranch_ : branch;

    // Check if branch exists
    if (!branchExists(targetBranch)) {
        return "";
    }

    // Generate a new version ID
    std::string versionId = generateVersionId();

    // Create version directory
    std::string versionPath = getVersionPath(versionId);
    fs::create_directories(versionPath);

    // Copy image from staging to version directory
    std::string versionImagePath = getVersionImagePath(versionId);
    fs::copy_file(stagingPath, versionImagePath, fs::copy_options::overwrite_existing);

    // Load the image and create Merkle Tree
    imageProcessor_.loadImage(versionImagePath);
    imageProcessor_.convertToGrayscale();
    MerkleTree merkleTree = imageProcessor_.createMerkleTree();

    // Get parent ID (current head of the branch)
    std::string parentId = branches_[targetBranch].headVersionId;

    // Create version info
    VersionInfo versionInfo;
    versionInfo.id = versionId;
    versionInfo.parentId = parentId;
    versionInfo.message = message;
    versionInfo.timestamp = getCurrentTimestamp();
    versionInfo.rootHash = merkleTree.getRootHash();
    versionInfo.imagePath = versionImagePath;
    versionInfo.branch = targetBranch;
    versionInfo.isMergeCommit = false;
    versionInfo.mergeSourceId = "";
    versionInfo.isEncrypted = false;

    // Encrypt image if requested
    if (encrypt && securityInitialized_) {
        // Generate encryption key and IV
        std::string encryptionKey = Security::generateRandomKey();
        std::string encryptionIV = Security::generateRandomIV();

        // Encrypt the image
        if (encryptImage(versionImagePath, encryptionKey, encryptionIV)) {
            versionInfo.encryptionKey = encryptionKey;
            versionInfo.encryptionIV = encryptionIV;
            versionInfo.isEncrypted = true;
        }
    }

    // Sign version if requested
    if (sign && securityInitialized_) {
        versionInfo.signature = signVersion(versionInfo);
    }

    // Save version info
    if (!saveVersionInfo(versionInfo)) {
        return "";
    }

    // Update version map
    versions_[versionId] = versionInfo;

    // Update branch head
    branches_[targetBranch].headVersionId = versionId;
    saveBranchInfo(branches_[targetBranch]);

    // If committing to current branch, update current version
    if (targetBranch == currentBranch_) {
        currentVersionId_ = versionId;
        saveCurrentState();
    }

    // Clear staging area
    fs::remove(stagingPath);

    return versionId;
}

cv::Mat VersionControl::compareVersions(const std::string& versionId1, const std::string& versionId2) {
    // Check if versions exist
    if (!versionExists(versionId1) || !versionExists(versionId2)) {
        return cv::Mat();
    }

    // Get version info for both versions
    VersionInfo version1 = getVersion(versionId1);
    VersionInfo version2 = getVersion(versionId2);

    // Get images for both versions
    cv::Mat image1, image2;

    // Handle encrypted images
    if (version1.isEncrypted) {
        image1 = decryptImage(version1.imagePath, version1.encryptionKey, version1.encryptionIV);
    } else {
        image1 = cv::imread(version1.imagePath);
    }

    if (version2.isEncrypted) {
        image2 = decryptImage(version2.imagePath, version2.encryptionKey, version2.encryptionIV);
    } else {
        image2 = cv::imread(version2.imagePath);
    }

    if (image1.empty() || image2.empty()) {
        return cv::Mat();
    }

    // Compare images
    ImageProcessor processor;
    processor.loadImage(image1);
    return processor.compareImages(image2);
}

bool VersionControl::rollbackToVersion(const std::string& versionId, const std::string& branch) {
    // Check if version exists
    if (!versionExists(versionId)) {
        return false;
    }

    // Determine which branch to update
    std::string targetBranch = branch.empty() ? currentBranch_ : branch;

    // Check if branch exists
    if (!branchExists(targetBranch)) {
        return false;
    }

    // Update branch head
    branches_[targetBranch].headVersionId = versionId;
    saveBranchInfo(branches_[targetBranch]);

    // If rolling back current branch, update current version
    if (targetBranch == currentBranch_) {
        currentVersionId_ = versionId;
        saveCurrentState();
    }

    return true;
}

std::vector<VersionInfo> VersionControl::getAllVersions() const {
    std::vector<VersionInfo> allVersions;

    for (const auto& [id, info] : versions_) {
        allVersions.push_back(info);
    }

    return allVersions;
}

VersionInfo VersionControl::getVersion(const std::string& versionId) const {
    if (versionExists(versionId)) {
        return versions_.at(versionId);
    }

    return VersionInfo();
}

VersionInfo VersionControl::getCurrentVersion() const {
    if (!currentVersionId_.empty() && versionExists(currentVersionId_)) {
        return versions_.at(currentVersionId_);
    }

    return VersionInfo();
}

bool VersionControl::versionExists(const std::string& versionId) const {
    return versions_.find(versionId) != versions_.end();
}

cv::Mat VersionControl::getVersionImage(const std::string& versionId) const {
    if (!versionExists(versionId)) {
        return cv::Mat();
    }

    VersionInfo versionInfo = getVersion(versionId);

    // Handle encrypted images
    if (versionInfo.isEncrypted) {
        return decryptImage(versionInfo.imagePath, versionInfo.encryptionKey, versionInfo.encryptionIV);
    } else {
        return cv::imread(versionInfo.imagePath);
    }
}

std::string VersionControl::generateVersionId() const {
    // Generate a random UUID-like string
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    const char* hexChars = "0123456789abcdef";
    std::stringstream ss;

    for (int i = 0; i < 8; ++i) {
        ss << hexChars[dis(gen)];
    }
    ss << '-';

    for (int j = 0; j < 3; ++j) {
        for (int i = 0; i < 4; ++i) {
            ss << hexChars[dis(gen)];
        }
        ss << '-';
    }

    for (int i = 0; i < 12; ++i) {
        ss << hexChars[dis(gen)];
    }

    return ss.str();
}

std::string VersionControl::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");

    return ss.str();
}

bool VersionControl::saveVersionInfo(const VersionInfo& versionInfo) {
    // Create JSON object
    json j;
    j["id"] = versionInfo.id;
    j["parent_id"] = versionInfo.parentId;
    j["message"] = versionInfo.message;
    j["timestamp"] = versionInfo.timestamp;
    j["root_hash"] = versionInfo.rootHash;
    j["image_path"] = versionInfo.imagePath;
    j["branch"] = versionInfo.branch;
    j["is_merge_commit"] = versionInfo.isMergeCommit;
    j["merge_source_id"] = versionInfo.mergeSourceId;
    j["is_encrypted"] = versionInfo.isEncrypted;

    // Add encryption information if encrypted
    if (versionInfo.isEncrypted) {
        j["encryption_key"] = versionInfo.encryptionKey;
        j["encryption_iv"] = versionInfo.encryptionIV;
    }

    // Add signature if signed
    if (!versionInfo.signature.empty()) {
        // Convert binary signature to base64 for storage
        std::stringstream ss;
        for (const auto& byte : versionInfo.signature) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        j["signature"] = ss.str();
    }

    // Save to file
    std::string infoPath = getVersionPath(versionInfo.id) + "/info.json";
    std::ofstream file(infoPath);

    if (!file.is_open()) {
        return false;
    }

    file << j.dump(4);
    file.close();

    return true;
}

bool VersionControl::loadVersionInfo() {
    // Clear existing versions
    versions_.clear();

    // Check if versions directory exists
    std::string versionsPath = repoPath_ + "/versions";
    if (!fs::exists(versionsPath)) {
        return false;
    }

    // Iterate through version directories
    for (const auto& entry : fs::directory_iterator(versionsPath)) {
        if (entry.is_directory()) {
            std::string infoPath = entry.path().string() + "/info.json";

            if (fs::exists(infoPath)) {
                std::ifstream file(infoPath);

                if (file.is_open()) {
                    json j;
                    file >> j;

                    VersionInfo info;
                    info.id = j["id"];
                    info.parentId = j["parent_id"];
                    info.message = j["message"];
                    info.timestamp = j["timestamp"];
                    info.rootHash = j["root_hash"];
                    info.imagePath = j["image_path"];

                    // Handle optional fields for backward compatibility
                    if (j.contains("branch")) {
                        info.branch = j["branch"];
                    } else {
                        info.branch = "main"; // Default to main branch
                    }

                    if (j.contains("is_merge_commit")) {
                        info.isMergeCommit = j["is_merge_commit"];
                    } else {
                        info.isMergeCommit = false;
                    }

                    if (j.contains("merge_source_id")) {
                        info.mergeSourceId = j["merge_source_id"];
                    } else {
                        info.mergeSourceId = "";
                    }

                    // Handle encryption information
                    if (j.contains("is_encrypted")) {
                        info.isEncrypted = j["is_encrypted"];

                        if (info.isEncrypted) {
                            if (j.contains("encryption_key") && j.contains("encryption_iv")) {
                                info.encryptionKey = j["encryption_key"];
                                info.encryptionIV = j["encryption_iv"];
                            } else {
                                // Missing encryption info, mark as not encrypted
                                info.isEncrypted = false;
                            }
                        }
                    } else {
                        info.isEncrypted = false;
                    }

                    // Handle signature
                    if (j.contains("signature")) {
                        std::string signatureHex = j["signature"];
                        info.signature.clear();

                        // Convert hex string to binary
                        for (size_t i = 0; i < signatureHex.length(); i += 2) {
                            std::string byteString = signatureHex.substr(i, 2);
                            unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
                            info.signature.push_back(byte);
                        }
                    }

                    versions_[info.id] = info;
                }
            }
        }
    }

    // Load current state
    std::string statePath = repoPath_ + "/state.json";
    if (fs::exists(statePath)) {
        std::ifstream file(statePath);

        if (file.is_open()) {
            json j;
            file >> j;

            currentVersionId_ = j["current_version"];

            // Handle optional fields for backward compatibility
            if (j.contains("current_branch")) {
                currentBranch_ = j["current_branch"];
            } else {
                currentBranch_ = "main"; // Default to main branch
            }
        }
    }

    return true;
}

bool VersionControl::saveCurrentState() {
    // Create JSON object
    json j;
    j["current_version"] = currentVersionId_;
    j["current_branch"] = currentBranch_;

    // Save to file
    std::string statePath = repoPath_ + "/state.json";
    std::ofstream file(statePath);

    if (!file.is_open()) {
        return false;
    }

    file << j.dump(4);
    file.close();

    return true;
}

std::string VersionControl::getVersionPath(const std::string& versionId) const {
    return repoPath_ + "/versions/" + versionId;
}

std::string VersionControl::getVersionImagePath(const std::string& versionId) const {
    return getVersionPath(versionId) + "/image.png";
}

bool VersionControl::initSecurity() {
    // Check if security directory exists
    std::string securityPath = repoPath_ + "/security";
    if (!fs::exists(securityPath)) {
        fs::create_directories(securityPath);
    }

    // Check if keys exist
    std::string privateKeyPath = securityPath + "/private_key.pem";
    std::string publicKeyPath = securityPath + "/public_key.pem";

    if (!fs::exists(privateKeyPath) || !fs::exists(publicKeyPath)) {
        // Generate new key pair
        if (!security_.generateKeyPair(privateKeyPath, publicKeyPath)) {
            return false;
        }
    } else {
        // Load existing keys
        if (!security_.loadKeys(privateKeyPath, publicKeyPath)) {
            return false;
        }
    }

    securityInitialized_ = true;
    return true;
}

bool VersionControl::encryptImage(const std::string& imagePath, const std::string& encryptionKey, const std::string& encryptionIV) {
    // Read image file
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Read file content
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Encrypt data
    std::vector<unsigned char> encryptedData = security_.encrypt(data, encryptionKey, encryptionIV);
    if (encryptedData.empty()) {
        return false;
    }

    // Write encrypted data back to file
    std::ofstream outFile(imagePath, std::ios::binary);
    if (!outFile.is_open()) {
        return false;
    }

    outFile.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());
    outFile.close();

    return true;
}

cv::Mat VersionControl::decryptImage(const std::string& imagePath, const std::string& encryptionKey, const std::string& encryptionIV) const {
    // Read encrypted file
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        return cv::Mat();
    }

    // Read file content
    std::vector<unsigned char> encryptedData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Decrypt data
    std::vector<unsigned char> decryptedData = security_.decrypt(encryptedData, encryptionKey, encryptionIV);
    if (decryptedData.empty()) {
        return cv::Mat();
    }

    // Convert to cv::Mat
    cv::Mat image = cv::imdecode(decryptedData, cv::IMREAD_UNCHANGED);
    return image;
}

std::vector<unsigned char> VersionControl::signVersion(const VersionInfo& versionInfo) const {
    if (!securityInitialized_) {
        return {};
    }

    // Create data to sign (concatenate important fields)
    std::string dataStr = versionInfo.id + versionInfo.parentId + versionInfo.message +
                        versionInfo.timestamp + versionInfo.rootHash + versionInfo.branch;

    // Convert to binary
    std::vector<unsigned char> data(dataStr.begin(), dataStr.end());

    // Sign data
    return security_.sign(data);
}

bool VersionControl::verifyVersionSignature(const VersionInfo& versionInfo) const {
    if (!securityInitialized_ || versionInfo.signature.empty()) {
        return false;
    }

    // Create data to verify (concatenate important fields)
    std::string dataStr = versionInfo.id + versionInfo.parentId + versionInfo.message +
                        versionInfo.timestamp + versionInfo.rootHash + versionInfo.branch;

    // Convert to binary
    std::vector<unsigned char> data(dataStr.begin(), dataStr.end());

    // Verify signature
    return security_.verify(data, versionInfo.signature);
}

bool VersionControl::saveBranchInfo(const BranchInfo& branchInfo) {
    // Create JSON object
    json j;
    j["name"] = branchInfo.name;
    j["head_version_id"] = branchInfo.headVersionId;
    j["description"] = branchInfo.description;
    j["creation_timestamp"] = branchInfo.creationTimestamp;

    // Save to file
    std::string branchPath = repoPath_ + "/branches/" + branchInfo.name + ".json";
    std::ofstream file(branchPath);

    if (!file.is_open()) {
        return false;
    }

    file << j.dump(4);
    file.close();

    return true;
}

bool VersionControl::loadBranchInfo() {
    // Clear existing branches
    branches_.clear();

    // Check if branches directory exists
    std::string branchesPath = repoPath_ + "/branches";
    if (!fs::exists(branchesPath)) {
        // Create branches directory if it doesn't exist
        fs::create_directories(branchesPath);

        // Create main branch if no branches exist
        BranchInfo mainBranch;
        mainBranch.name = "main";
        mainBranch.headVersionId = currentVersionId_;
        mainBranch.description = "Main branch";
        mainBranch.creationTimestamp = getCurrentTimestamp();

        branches_["main"] = mainBranch;
        saveBranchInfo(mainBranch);

        return true;
    }

    // Iterate through branch files
    for (const auto& entry : fs::directory_iterator(branchesPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::ifstream file(entry.path());

            if (file.is_open()) {
                json j;
                file >> j;

                BranchInfo info;
                info.name = j["name"];
                info.headVersionId = j["head_version_id"];
                info.description = j["description"];
                info.creationTimestamp = j["creation_timestamp"];

                branches_[info.name] = info;
            }
        }
    }

    return true;
}

bool VersionControl::createBranch(const std::string& branchName, const std::string& startPoint, const std::string& description) {
    // Check if branch already exists
    if (branchExists(branchName)) {
        return false;
    }

    // Determine start point
    std::string startVersionId = startPoint.empty() ? currentVersionId_ : startPoint;

    // Check if start point exists
    if (!startVersionId.empty() && !versionExists(startVersionId)) {
        return false;
    }

    // Create branch info
    BranchInfo branchInfo;
    branchInfo.name = branchName;
    branchInfo.headVersionId = startVersionId;
    branchInfo.description = description;
    branchInfo.creationTimestamp = getCurrentTimestamp();

    // Save branch info
    if (!saveBranchInfo(branchInfo)) {
        return false;
    }

    // Add to branches map
    branches_[branchName] = branchInfo;

    return true;
}

bool VersionControl::switchBranch(const std::string& branchName) {
    // Check if branch exists
    if (!branchExists(branchName)) {
        return false;
    }

    // Update current branch
    currentBranch_ = branchName;

    // Update current version to branch head
    currentVersionId_ = branches_[branchName].headVersionId;

    // Save current state
    return saveCurrentState();
}

std::vector<BranchInfo> VersionControl::getAllBranches() const {
    std::vector<BranchInfo> allBranches;

    for (const auto& [name, info] : branches_) {
        allBranches.push_back(info);
    }

    return allBranches;
}

BranchInfo VersionControl::getCurrentBranch() const {
    if (branchExists(currentBranch_)) {
        return branches_.at(currentBranch_);
    }

    return BranchInfo();
}

bool VersionControl::branchExists(const std::string& branchName) const {
    return branches_.find(branchName) != branches_.end();
}

std::string VersionControl::findCommonAncestor(const std::string& versionId1, const std::string& versionId2) const {
    // Check if versions exist
    if (!versionExists(versionId1) || !versionExists(versionId2)) {
        return "";
    }

    // If one of the versions is the same, it's the common ancestor
    if (versionId1 == versionId2) {
        return versionId1;
    }

    // Build ancestry path for first version
    std::set<std::string> ancestry1;
    std::string current = versionId1;

    while (!current.empty()) {
        ancestry1.insert(current);
        current = versions_.at(current).parentId;
    }

    // Find first common ancestor in second version's ancestry
    current = versionId2;

    while (!current.empty()) {
        if (ancestry1.find(current) != ancestry1.end()) {
            return current;
        }
        current = versions_.at(current).parentId;
    }

    return "";
}

cv::Mat VersionControl::mergeImages(const cv::Mat& baseImage, const cv::Mat& ourImage, const cv::Mat& theirImage) const {
    // Simple merge strategy: take non-conflicting changes from both images
    cv::Mat mergedImage = baseImage.clone();

    // Create masks for changes in each image
    cv::Mat ourDiff, theirDiff;
    cv::absdiff(baseImage, ourImage, ourDiff);
    cv::absdiff(baseImage, theirImage, theirDiff);

    // Threshold to get binary masks
    cv::Mat ourMask, theirMask;
    cv::threshold(ourDiff, ourMask, 10, 255, cv::THRESH_BINARY);
    cv::threshold(theirDiff, theirMask, 10, 255, cv::THRESH_BINARY);

    // Find conflict regions (where both images changed)
    cv::Mat conflictMask;
    cv::bitwise_and(ourMask, theirMask, conflictMask);

    // Apply non-conflicting changes from our image
    cv::Mat ourNonConflict;
    cv::bitwise_xor(ourMask, conflictMask, ourNonConflict);
    ourImage.copyTo(mergedImage, ourNonConflict);

    // Apply non-conflicting changes from their image
    cv::Mat theirNonConflict;
    cv::bitwise_xor(theirMask, conflictMask, theirNonConflict);
    theirImage.copyTo(mergedImage, theirNonConflict);

    // For conflicts, use a visual indicator (red color)
    if (mergedImage.channels() == 1) {
        cv::cvtColor(mergedImage, mergedImage, cv::COLOR_GRAY2BGR);
    }

    // Mark conflict regions in red
    for (int y = 0; y < mergedImage.rows; y++) {
        for (int x = 0; x < mergedImage.cols; x++) {
            if (conflictMask.at<uchar>(y, x) > 0) {
                mergedImage.at<cv::Vec3b>(y, x)[0] = 0;   // B
                mergedImage.at<cv::Vec3b>(y, x)[1] = 0;   // G
                mergedImage.at<cv::Vec3b>(y, x)[2] = 255; // R
            }
        }
    }

    return mergedImage;
}

std::string VersionControl::mergeBranch(const std::string& branchName, const std::string& message) {
    // Check if branch exists
    if (!branchExists(branchName)) {
        return "";
    }

    // Check if current branch exists
    if (!branchExists(currentBranch_)) {
        return "";
    }

    // Get head versions of both branches
    std::string ourVersionId = branches_[currentBranch_].headVersionId;
    std::string theirVersionId = branches_[branchName].headVersionId;

    // Find common ancestor
    std::string baseVersionId = findCommonAncestor(ourVersionId, theirVersionId);

    if (baseVersionId.empty()) {
        return "";
    }

    // If their version is already an ancestor of our version, no merge needed
    if (baseVersionId == theirVersionId) {
        return ourVersionId;
    }

    // If our version is an ancestor of their version, fast-forward
    if (baseVersionId == ourVersionId) {
        // Update current branch head
        branches_[currentBranch_].headVersionId = theirVersionId;
        saveBranchInfo(branches_[currentBranch_]);

        // Update current version
        currentVersionId_ = theirVersionId;
        saveCurrentState();

        return theirVersionId;
    }

    // Get images for base, ours, and theirs
    cv::Mat baseImage = getVersionImage(baseVersionId);
    cv::Mat ourImage = getVersionImage(ourVersionId);
    cv::Mat theirImage = getVersionImage(theirVersionId);

    if (baseImage.empty() || ourImage.empty() || theirImage.empty()) {
        return "";
    }

    // Merge images
    cv::Mat mergedImage = mergeImages(baseImage, ourImage, theirImage);

    // Save merged image to staging
    std::string stagingPath = repoPath_ + "/staging/image.png";
    cv::imwrite(stagingPath, mergedImage);

    // Create merge commit
    std::string mergeMessage = message.empty() ?
        "Merge branch '" + branchName + "' into " + currentBranch_ :
        message;

    // Generate a new version ID
    std::string versionId = generateVersionId();

    // Create version directory
    std::string versionPath = getVersionPath(versionId);
    fs::create_directories(versionPath);

    // Copy image from staging to version directory
    std::string versionImagePath = getVersionImagePath(versionId);
    fs::copy_file(stagingPath, versionImagePath, fs::copy_options::overwrite_existing);

    // Load the image and create Merkle Tree
    imageProcessor_.loadImage(versionImagePath);
    imageProcessor_.convertToGrayscale();
    MerkleTree merkleTree = imageProcessor_.createMerkleTree();

    // Create version info
    VersionInfo versionInfo;
    versionInfo.id = versionId;
    versionInfo.parentId = ourVersionId;
    versionInfo.message = mergeMessage;
    versionInfo.timestamp = getCurrentTimestamp();
    versionInfo.rootHash = merkleTree.getRootHash();
    versionInfo.imagePath = versionImagePath;
    versionInfo.branch = currentBranch_;
    versionInfo.isMergeCommit = true;
    versionInfo.mergeSourceId = theirVersionId;

    // Save version info
    if (!saveVersionInfo(versionInfo)) {
        return "";
    }

    // Update version map
    versions_[versionId] = versionInfo;

    // Update branch head
    branches_[currentBranch_].headVersionId = versionId;
    saveBranchInfo(branches_[currentBranch_]);

    // Update current version
    currentVersionId_ = versionId;
    saveCurrentState();

    // Clear staging area
    fs::remove(stagingPath);

    return versionId;
}

bool VersionControl::deleteBranch(const std::string& branchName) {
    // Check if branch exists
    if (!branchExists(branchName)) {
        return false;
    }

    // Cannot delete current branch
    if (branchName == currentBranch_) {
        return false;
    }

    // Cannot delete main branch
    if (branchName == "main") {
        return false;
    }

    // Remove branch file
    std::string branchPath = repoPath_ + "/branches/" + branchName + ".json";
    if (fs::exists(branchPath)) {
        fs::remove(branchPath);
    }

    // Remove from branches map
    branches_.erase(branchName);

    return true;
}
