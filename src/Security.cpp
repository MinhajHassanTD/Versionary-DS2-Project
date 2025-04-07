#include "Security.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <openssl/rand.h>
#include <openssl/sha.h>

Security::Security() : privateKey_(nullptr), publicKey_(nullptr) {
    initOpenSSL();
}

Security::~Security() {
    if (privateKey_) {
        EVP_PKEY_free(privateKey_);
    }
    
    if (publicKey_) {
        EVP_PKEY_free(publicKey_);
    }
    
    cleanupOpenSSL();
}

void Security::initOpenSSL() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

void Security::cleanupOpenSSL() {
    // Clean up OpenSSL
    EVP_cleanup();
    ERR_free_strings();
}

bool Security::generateKeyPair(const std::string& privateKeyPath, const std::string& publicKeyPath) {
    // Create a new RSA key pair
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        return false;
    }
    
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    
    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    
    EVP_PKEY_CTX_free(ctx);
    
    // Save private key to file
    FILE* privateKeyFile = fopen(privateKeyPath.c_str(), "wb");
    if (!privateKeyFile) {
        EVP_PKEY_free(pkey);
        return false;
    }
    
    if (!PEM_write_PrivateKey(privateKeyFile, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
        fclose(privateKeyFile);
        EVP_PKEY_free(pkey);
        return false;
    }
    
    fclose(privateKeyFile);
    
    // Save public key to file
    FILE* publicKeyFile = fopen(publicKeyPath.c_str(), "wb");
    if (!publicKeyFile) {
        EVP_PKEY_free(pkey);
        return false;
    }
    
    if (!PEM_write_PUBKEY(publicKeyFile, pkey)) {
        fclose(publicKeyFile);
        EVP_PKEY_free(pkey);
        return false;
    }
    
    fclose(publicKeyFile);
    
    // Store keys
    if (privateKey_) {
        EVP_PKEY_free(privateKey_);
    }
    
    if (publicKey_) {
        EVP_PKEY_free(publicKey_);
    }
    
    privateKey_ = pkey;
    publicKey_ = pkey;
    
    return true;
}

bool Security::loadKeys(const std::string& privateKeyPath, const std::string& publicKeyPath) {
    // Load private key
    FILE* privateKeyFile = fopen(privateKeyPath.c_str(), "rb");
    if (!privateKeyFile) {
        return false;
    }
    
    EVP_PKEY* privateKey = PEM_read_PrivateKey(privateKeyFile, nullptr, nullptr, nullptr);
    fclose(privateKeyFile);
    
    if (!privateKey) {
        return false;
    }
    
    // Load public key
    FILE* publicKeyFile = fopen(publicKeyPath.c_str(), "rb");
    if (!publicKeyFile) {
        EVP_PKEY_free(privateKey);
        return false;
    }
    
    EVP_PKEY* publicKey = PEM_read_PUBKEY(publicKeyFile, nullptr, nullptr, nullptr);
    fclose(publicKeyFile);
    
    if (!publicKey) {
        EVP_PKEY_free(privateKey);
        return false;
    }
    
    // Store keys
    if (privateKey_) {
        EVP_PKEY_free(privateKey_);
    }
    
    if (publicKey_) {
        EVP_PKEY_free(publicKey_);
    }
    
    privateKey_ = privateKey;
    publicKey_ = publicKey;
    
    return true;
}

std::vector<unsigned char> Security::encrypt(const std::vector<unsigned char>& data, 
                                          const std::string& key, 
                                          const std::string& iv) {
    // Create cipher context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return {};
    }
    
    // Initialize encryption operation
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, 
                         reinterpret_cast<const unsigned char*>(key.c_str()), 
                         reinterpret_cast<const unsigned char*>(iv.c_str())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    
    // Allocate output buffer
    std::vector<unsigned char> encryptedData(data.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int outLen1 = 0;
    
    // Encrypt data
    if (EVP_EncryptUpdate(ctx, encryptedData.data(), &outLen1, 
                        data.data(), static_cast<int>(data.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    
    int outLen2 = 0;
    
    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, encryptedData.data() + outLen1, &outLen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Resize output buffer to actual size
    encryptedData.resize(outLen1 + outLen2);
    
    return encryptedData;
}

std::vector<unsigned char> Security::decrypt(const std::vector<unsigned char>& encryptedData, 
                                          const std::string& key, 
                                          const std::string& iv) {
    // Create cipher context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return {};
    }
    
    // Initialize decryption operation
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, 
                         reinterpret_cast<const unsigned char*>(key.c_str()), 
                         reinterpret_cast<const unsigned char*>(iv.c_str())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    
    // Allocate output buffer
    std::vector<unsigned char> decryptedData(encryptedData.size());
    int outLen1 = 0;
    
    // Decrypt data
    if (EVP_DecryptUpdate(ctx, decryptedData.data(), &outLen1, 
                        encryptedData.data(), static_cast<int>(encryptedData.size())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    
    int outLen2 = 0;
    
    // Finalize decryption
    if (EVP_DecryptFinal_ex(ctx, decryptedData.data() + outLen1, &outLen2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return {};
    }
    
    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    
    // Resize output buffer to actual size
    decryptedData.resize(outLen1 + outLen2);
    
    return decryptedData;
}

std::vector<unsigned char> Security::sign(const std::vector<unsigned char>& data) {
    if (!privateKey_) {
        return {};
    }
    
    // Create message digest context
    EVP_MD_CTX* mdctx = EVP_MD_CTX_create();
    if (!mdctx) {
        return {};
    }
    
    // Initialize signing operation
    if (EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, privateKey_) != 1) {
        EVP_MD_CTX_destroy(mdctx);
        return {};
    }
    
    // Update with data
    if (EVP_DigestSignUpdate(mdctx, data.data(), data.size()) != 1) {
        EVP_MD_CTX_destroy(mdctx);
        return {};
    }
    
    // Get signature length
    size_t sigLen = 0;
    if (EVP_DigestSignFinal(mdctx, nullptr, &sigLen) != 1) {
        EVP_MD_CTX_destroy(mdctx);
        return {};
    }
    
    // Allocate signature buffer
    std::vector<unsigned char> signature(sigLen);
    
    // Get signature
    if (EVP_DigestSignFinal(mdctx, signature.data(), &sigLen) != 1) {
        EVP_MD_CTX_destroy(mdctx);
        return {};
    }
    
    // Clean up
    EVP_MD_CTX_destroy(mdctx);
    
    // Resize signature buffer to actual size
    signature.resize(sigLen);
    
    return signature;
}

bool Security::verify(const std::vector<unsigned char>& data, const std::vector<unsigned char>& signature) {
    if (!publicKey_) {
        return false;
    }
    
    // Create message digest context
    EVP_MD_CTX* mdctx = EVP_MD_CTX_create();
    if (!mdctx) {
        return false;
    }
    
    // Initialize verification operation
    if (EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, publicKey_) != 1) {
        EVP_MD_CTX_destroy(mdctx);
        return false;
    }
    
    // Update with data
    if (EVP_DigestVerifyUpdate(mdctx, data.data(), data.size()) != 1) {
        EVP_MD_CTX_destroy(mdctx);
        return false;
    }
    
    // Verify signature
    int result = EVP_DigestVerifyFinal(mdctx, signature.data(), signature.size());
    
    // Clean up
    EVP_MD_CTX_destroy(mdctx);
    
    return result == 1;
}

std::string Security::generateRandomKey(int length) {
    std::vector<unsigned char> key(length);
    
    // Generate random bytes
    if (RAND_bytes(key.data(), length) != 1) {
        // Fallback to C++ random if OpenSSL fails
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        for (int i = 0; i < length; ++i) {
            key[i] = static_cast<unsigned char>(dis(gen));
        }
    }
    
    // Convert to hex string
    std::stringstream ss;
    for (int i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(key[i]);
    }
    
    return ss.str();
}

std::string Security::generateRandomIV() {
    // AES-256-CBC uses a 16-byte IV
    return generateRandomKey(16);
}

std::string Security::computeHash(const std::vector<unsigned char>& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), data.size());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}
