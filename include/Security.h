#ifndef SECURITY_H
#define SECURITY_H

#include <string>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

/**
 * @brief Security class for encryption and digital signatures
 * 
 * This class provides functionality for encrypting and decrypting
 * data, as well as creating and verifying digital signatures.
 */
class Security {
public:
    /**
     * @brief Construct a new Security object
     */
    Security();
    
    /**
     * @brief Destroy the Security object
     */
    ~Security();
    
    /**
     * @brief Generate a new key pair
     * 
     * @param privateKeyPath Path to save the private key
     * @param publicKeyPath Path to save the public key
     * @return bool True if key generation was successful
     */
    bool generateKeyPair(const std::string& privateKeyPath, const std::string& publicKeyPath);
    
    /**
     * @brief Load keys from files
     * 
     * @param privateKeyPath Path to the private key
     * @param publicKeyPath Path to the public key
     * @return bool True if key loading was successful
     */
    bool loadKeys(const std::string& privateKeyPath, const std::string& publicKeyPath);
    
    /**
     * @brief Encrypt data
     * 
     * @param data Data to encrypt
     * @param key Encryption key
     * @param iv Initialization vector
     * @return std::vector<unsigned char> Encrypted data
     */
    std::vector<unsigned char> encrypt(const std::vector<unsigned char>& data, 
                                     const std::string& key, 
                                     const std::string& iv);
    
    /**
     * @brief Decrypt data
     * 
     * @param encryptedData Encrypted data
     * @param key Encryption key
     * @param iv Initialization vector
     * @return std::vector<unsigned char> Decrypted data
     */
    std::vector<unsigned char> decrypt(const std::vector<unsigned char>& encryptedData, 
                                     const std::string& key, 
                                     const std::string& iv);
    
    /**
     * @brief Sign data
     * 
     * @param data Data to sign
     * @return std::vector<unsigned char> Signature
     */
    std::vector<unsigned char> sign(const std::vector<unsigned char>& data);
    
    /**
     * @brief Verify signature
     * 
     * @param data Data to verify
     * @param signature Signature to verify
     * @return bool True if signature is valid
     */
    bool verify(const std::vector<unsigned char>& data, const std::vector<unsigned char>& signature);
    
    /**
     * @brief Generate a random key
     * 
     * @param length Key length
     * @return std::string Random key
     */
    static std::string generateRandomKey(int length = 32);
    
    /**
     * @brief Generate a random initialization vector
     * 
     * @return std::string Random initialization vector
     */
    static std::string generateRandomIV();
    
    /**
     * @brief Compute SHA-256 hash
     * 
     * @param data Data to hash
     * @return std::string Hash
     */
    static std::string computeHash(const std::vector<unsigned char>& data);

private:
    EVP_PKEY* privateKey_;
    EVP_PKEY* publicKey_;
    
    /**
     * @brief Initialize OpenSSL
     */
    void initOpenSSL();
    
    /**
     * @brief Clean up OpenSSL
     */
    void cleanupOpenSSL();
};

#endif // SECURITY_H
