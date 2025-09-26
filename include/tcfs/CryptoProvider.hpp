#pragma once

#include "Errors.hpp"
#include "Policy.hpp"
#include <vector>
#include <memory>
#include <cstdint>

namespace tcfs {

/**
 * @brief Cryptographic key material
 */
struct CryptoKey {
    std::vector<uint8_t> data;
    
    CryptoKey() = default;
    explicit CryptoKey(std::vector<uint8_t> key_data) : data(std::move(key_data)) {}
    explicit CryptoKey(size_t size) : data(size) {}
    
    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    void clear() { 
        // Secure clear
        if (!data.empty()) {
            std::fill(data.begin(), data.end(), 0);
            data.clear();
        }
    }
    
    ~CryptoKey() { clear(); }
    
    // Move semantics
    CryptoKey(CryptoKey&& other) noexcept : data(std::move(other.data)) {}
    CryptoKey& operator=(CryptoKey&& other) noexcept {
        if (this != &other) {
            clear();
            data = std::move(other.data);
        }
        return *this;
    }
    
    // Disable copy
    CryptoKey(const CryptoKey&) = delete;
    CryptoKey& operator=(const CryptoKey&) = delete;
};

/**
 * @brief Initialization Vector for encryption
 */
using CryptoIV = std::vector<uint8_t>;

/**
 * @brief Authentication tag for AEAD
 */
using AuthTag = std::vector<uint8_t>;

/**
 * @brief Salt for key derivation
 */
using CryptoSalt = std::vector<uint8_t>;

/**
 * @brief Encrypted data container
 */
struct EncryptedData {
    std::vector<uint8_t> ciphertext;
    CryptoIV iv;
    AuthTag tag;
    
    EncryptedData() = default;
    EncryptedData(std::vector<uint8_t> ct, CryptoIV initialization_vector, AuthTag auth_tag)
        : ciphertext(std::move(ct)), iv(std::move(initialization_vector)), tag(std::move(auth_tag)) {}
};

/**
 * @brief Key derivation parameters
 */
struct KDFParams {
    KDFType type;
    CryptoSalt salt;
    uint32_t iterations = 0;  // For PBKDF2
    uint32_t memory_kb = 0;   // For Argon2
    uint32_t parallelism = 0; // For Argon2
    
    KDFParams() = default;
    explicit KDFParams(KDFType kdf_type) : type(kdf_type) {}
};

/**
 * @brief Abstract cryptographic provider interface
 */
class CryptoProvider {
public:
    virtual ~CryptoProvider() = default;
    
    // Key generation
    virtual CryptoKey generateKey() = 0;
    virtual CryptoIV generateIV() = 0;
    virtual CryptoSalt generateSalt() = 0;
    
    // Key derivation
    virtual CryptoKey deriveKey(const std::string& password, const CryptoSalt& salt, const KDFParams& params) = 0;
    
    // Encryption/Decryption
    virtual EncryptedData encrypt(const std::vector<uint8_t>& plaintext, const CryptoKey& key, const CryptoIV& iv) = 0;
    virtual std::vector<uint8_t> decrypt(const EncryptedData& encrypted, const CryptoKey& key, const CryptoIV& iv) = 0;
    
    // Hashing
    virtual std::vector<uint8_t> sha256(const std::vector<uint8_t>& data) = 0;
    
    // Utility
    virtual std::string toHex(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> fromHex(const std::string& hex) = 0;
    virtual std::string toBase64(const std::vector<uint8_t>& data) = 0;
    virtual std::vector<uint8_t> fromBase64(const std::string& base64) = 0;
    
    // Constants
    static constexpr size_t AES_256_KEY_SIZE = 32;
    static constexpr size_t AES_GCM_IV_SIZE = 12;
    static constexpr size_t AES_GCM_TAG_SIZE = 16;
    static constexpr size_t SHA256_DIGEST_SIZE = 32;
    static constexpr size_t DEFAULT_SALT_SIZE = 32;
};

/**
 * @brief OpenSSL implementation of CryptoProvider
 */
class OpenSSLCryptoProvider : public CryptoProvider {
public:
    OpenSSLCryptoProvider();
    ~OpenSSLCryptoProvider() override;
    
    // Implement CryptoProvider interface
    CryptoKey generateKey() override;
    CryptoIV generateIV() override;
    CryptoSalt generateSalt() override;
    
    CryptoKey deriveKey(const std::string& password, const CryptoSalt& salt, const KDFParams& params) override;
    
    EncryptedData encrypt(const std::vector<uint8_t>& plaintext, const CryptoKey& key, const CryptoIV& iv) override;
    std::vector<uint8_t> decrypt(const EncryptedData& encrypted, const CryptoKey& key, const CryptoIV& iv) override;
    
    std::vector<uint8_t> sha256(const std::vector<uint8_t>& data) override;
    
    std::string toHex(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> fromHex(const std::string& hex) override;
    std::string toBase64(const std::vector<uint8_t>& data) override;
    std::vector<uint8_t> fromBase64(const std::string& base64) override;

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

/**
 * @brief Factory function to create a crypto provider
 */
std::unique_ptr<CryptoProvider> createCryptoProvider();

} // namespace tcfs