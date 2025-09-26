#include <tcfs/CryptoProvider.hpp>
#include <tcfs/Errors.hpp>

#if TCFS_HAS_OPENSSL
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#endif

#include <stdexcept>
#include <memory>
#include <iomanip>
#include <sstream>
#include <random>
#include <algorithm>

namespace tcfs {

#if TCFS_HAS_OPENSSL

// Implementation class for OpenSSLCryptoProvider
class OpenSSLCryptoProvider::Impl {
public:
    void handleOpenSSLError(const std::string& operation) {
        unsigned long err = ERR_get_error();
        char err_buf[256];
        ERR_error_string_n(err, err_buf, sizeof(err_buf));
        throw TCFSException(ErrorCode::CRYPTO_ERROR, operation + ": " + err_buf);
    }
};

OpenSSLCryptoProvider::OpenSSLCryptoProvider() : pimpl_(std::make_unique<Impl>()) {
    // Initialize OpenSSL
    // Note: In OpenSSL 1.1.0+, initialization is automatic
    // but we can still call these for compatibility
}

OpenSSLCryptoProvider::~OpenSSLCryptoProvider() {
    // Cleanup is automatic in OpenSSL 1.1.0+
}

CryptoKey OpenSSLCryptoProvider::generateKey() {
    CryptoKey key;
    key.data.resize(32); // AES-256 key size
    
    if (RAND_bytes(key.data.data(), static_cast<int>(key.data.size())) != 1) {
        pimpl_->handleOpenSSLError("Key generation failed");
    }
    
    return key;
}

CryptoIV OpenSSLCryptoProvider::generateIV() {
    CryptoIV iv;
    iv.resize(12); // GCM IV size
    
    if (RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1) {
        pimpl_->handleOpenSSLError("IV generation failed");
    }
    
    return iv;
}

CryptoSalt OpenSSLCryptoProvider::generateSalt() {
    CryptoSalt salt;
    salt.resize(16);
    
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) {
        pimpl_->handleOpenSSLError("Salt generation failed");
    }
    
    return salt;
}

CryptoKey OpenSSLCryptoProvider::deriveKey(const std::string& password, const CryptoSalt& salt, const KDFParams& params) {
    CryptoKey derived_key;
    derived_key.data.resize(32); // AES-256 key size
    
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()),
                          salt.data(), static_cast<int>(salt.size()),
                          static_cast<int>(params.iterations),
                          EVP_sha256(),
                          static_cast<int>(derived_key.data.size()),
                          derived_key.data.data()) != 1) {
        pimpl_->handleOpenSSLError("Key derivation failed");
    }
    
    return derived_key;
}

EncryptedData OpenSSLCryptoProvider::encrypt(const std::vector<uint8_t>& plaintext, const CryptoKey& key, const CryptoIV& iv) {
    EncryptedData result;
    result.iv = iv; // Set the IV in the result
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        pimpl_->handleOpenSSLError("Failed to create cipher context");
    }
    
    try {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data.data(), iv.data()) != 1) {
            pimpl_->handleOpenSSLError("Failed to initialize encryption");
        }
        
        result.ciphertext.resize(plaintext.size());
        int len;
        
        if (EVP_EncryptUpdate(ctx, result.ciphertext.data(), &len, plaintext.data(), static_cast<int>(plaintext.size())) != 1) {
            pimpl_->handleOpenSSLError("Failed to encrypt data");
        }
        
        int final_len;
        if (EVP_EncryptFinal_ex(ctx, result.ciphertext.data() + len, &final_len) != 1) {
            pimpl_->handleOpenSSLError("Failed to finalize encryption");
        }
        
        result.ciphertext.resize(len + final_len);
        
        // Get the tag
        result.tag.resize(16);
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, result.tag.data()) != 1) {
            pimpl_->handleOpenSSLError("Failed to get authentication tag");
        }
        
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    return result;
}

std::vector<uint8_t> OpenSSLCryptoProvider::decrypt(const EncryptedData& encrypted, const CryptoKey& key, const CryptoIV& iv) {
    std::vector<uint8_t> plaintext;
    
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        pimpl_->handleOpenSSLError("Failed to create cipher context");
    }
    
    try {
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data.data(), iv.data()) != 1) {
            pimpl_->handleOpenSSLError("Failed to initialize decryption");
        }
        
        // Set the tag BEFORE decryption for GCM mode
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, static_cast<int>(encrypted.tag.size()), 
                               const_cast<uint8_t*>(encrypted.tag.data())) != 1) {
            pimpl_->handleOpenSSLError("Failed to set authentication tag");
        }
        
        plaintext.resize(encrypted.ciphertext.size());
        int len;
        
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, encrypted.ciphertext.data(), static_cast<int>(encrypted.ciphertext.size())) != 1) {
            pimpl_->handleOpenSSLError("Failed to decrypt data");
        }
        
        int final_len;
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &final_len) != 1) {
            pimpl_->handleOpenSSLError("Failed to finalize decryption - authentication failed");
        }
        
        plaintext.resize(len + final_len);
        
    } catch (...) {
        EVP_CIPHER_CTX_free(ctx);
        throw;
    }
    
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}

std::vector<uint8_t> OpenSSLCryptoProvider::sha256(const std::vector<uint8_t>& data) {
    std::vector<uint8_t> hash(32);
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        pimpl_->handleOpenSSLError("Failed to create hash context");
    }
    
    try {
        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
            pimpl_->handleOpenSSLError("Failed to initialize hash");
        }
        
        if (!data.empty()) {
            if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
                pimpl_->handleOpenSSLError("Failed to update hash");
            }
        }
        
        unsigned int hash_len;
        if (EVP_DigestFinal_ex(ctx, hash.data(), &hash_len) != 1) {
            pimpl_->handleOpenSSLError("Failed to finalize hash");
        }
        
    } catch (...) {
        EVP_MD_CTX_free(ctx);
        throw;
    }
    
    EVP_MD_CTX_free(ctx);
    return hash;
}

std::string OpenSSLCryptoProvider::toHex(const std::vector<uint8_t>& data) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    for (uint8_t byte : data) {
        ss << std::setw(2) << static_cast<unsigned>(byte);
    }
    return ss.str();
}

std::vector<uint8_t> OpenSSLCryptoProvider::fromHex(const std::string& hex) {
    std::vector<uint8_t> data;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byte_str = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
        data.push_back(byte);
    }
    return data;
}

std::string OpenSSLCryptoProvider::toBase64(const std::vector<uint8_t>& data) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    
    if (!data.empty()) {
        BIO_write(bio, data.data(), static_cast<int>(data.size()));
    }
    BIO_flush(bio);
    
    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);
    
    std::string result(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(bio);
    
    return result;
}

std::vector<uint8_t> OpenSSLCryptoProvider::fromBase64(const std::string& base64) {
    BIO* bio = BIO_new_mem_buf(base64.c_str(), static_cast<int>(base64.length()));
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    
    std::vector<uint8_t> result(base64.length());
    int decoded_length = BIO_read(bio, result.data(), static_cast<int>(result.size()));
    
    BIO_free_all(bio);
    
    if (decoded_length < 0) {
        throw TCFSException(ErrorCode::CRYPTO_ERROR, "Base64 decoding failed");
    }
    
    result.resize(decoded_length);
    return result;
}

#else

// Mock implementation when OpenSSL is not available
class MockCryptoProvider : public CryptoProvider {
private:
    std::mt19937 rng{std::random_device{}()};

public:
    MockCryptoProvider() = default;
    CryptoKey generateKey() override {
        CryptoKey key;
        key.data.resize(32);
        std::generate(key.data.begin(), key.data.end(), [this]() { return static_cast<uint8_t>(rng()); });
        return key;
    }

    CryptoIV generateIV() override {
        CryptoIV iv;
        iv.resize(12);
        std::generate(iv.begin(), iv.end(), [this]() { return static_cast<uint8_t>(rng()); });
        return iv;
    }

    CryptoSalt generateSalt() override {
        CryptoSalt salt;
        salt.resize(16);
        std::generate(salt.begin(), salt.end(), [this]() { return static_cast<uint8_t>(rng()); });
        return salt;
    }

    CryptoKey deriveKey(const std::string& password, const CryptoSalt& salt, const KDFParams& params) override {
        // Simple mock - just hash password + salt
        CryptoKey key;
        key.data.resize(32);
        
        std::hash<std::string> hasher;
        std::string combined = password + std::string(salt.begin(), salt.end());
        size_t hash_value = hasher(combined);
        
        // Fill key with hash-based data
        for (size_t i = 0; i < key.data.size(); ++i) {
            key.data[i] = static_cast<uint8_t>((hash_value >> (i % 8)) & 0xFF);
        }
        
        return key;
    }

    EncryptedData encrypt(const std::vector<uint8_t>& plaintext, const CryptoKey& key, const CryptoIV& iv) override {
        // Simple XOR encryption (NOT SECURE - for demo only)
        EncryptedData result;
        result.ciphertext = plaintext;
        
        for (size_t i = 0; i < result.ciphertext.size(); ++i) {
            result.ciphertext[i] ^= key.data[i % key.data.size()];
            result.ciphertext[i] ^= iv[i % iv.size()];
        }
        
        // Generate authentication tag based on key (simple hash for mock)
        result.tag.resize(16);
        std::fill(result.tag.begin(), result.tag.end(), 0);
        for (size_t i = 0; i < key.data.size(); ++i) {
            result.tag[i % 16] ^= key.data[i];
        }
        
        return result;
    }

    std::vector<uint8_t> decrypt(const EncryptedData& encrypted, const CryptoKey& key, const CryptoIV& iv) override {
        // Verify authentication tag
        std::vector<uint8_t> expected_tag(16, 0);
        for (size_t i = 0; i < key.data.size(); ++i) {
            expected_tag[i % 16] ^= key.data[i];
        }
        
        if (encrypted.tag != expected_tag) {
            throw TCFSException(ErrorCode::CRYPTO_ERROR, "Authentication failed - wrong key");
        }
        
        // Simple XOR decryption (same as encryption for XOR)
        std::vector<uint8_t> result = encrypted.ciphertext;
        
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] ^= key.data[i % key.data.size()];
            result[i] ^= iv[i % iv.size()];
        }
        
        return result;
    }

    std::vector<uint8_t> sha256(const std::vector<uint8_t>& data) override {
        // Simple mock hash
        std::vector<uint8_t> hash(32);
        std::hash<std::string> hasher;
        std::string str_data(data.begin(), data.end());
        size_t hash_value = hasher(str_data);
        
        for (size_t i = 0; i < hash.size(); ++i) {
            hash[i] = static_cast<uint8_t>((hash_value >> (i % 8)) & 0xFF);
        }
        
        return hash;
    }

    std::string toHex(const std::vector<uint8_t>& data) override {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (uint8_t byte : data) {
            ss << std::setw(2) << static_cast<unsigned>(byte);
        }
        return ss.str();
    }

    std::vector<uint8_t> fromHex(const std::string& hex) override {
        std::vector<uint8_t> data;
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byte_str = hex.substr(i, 2);
            uint8_t byte = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
            data.push_back(byte);
        }
        return data;
    }

    std::string toBase64(const std::vector<uint8_t>& data) override {
        // Simple mock base64 (not real base64)
        if (data.empty()) {
            return "";
        }
        return toHex(data) + "_MOCK_B64";
    }

    std::vector<uint8_t> fromBase64(const std::string& base64) override {
        // Simple mock base64 decode
        if (base64.length() < 9 || base64.substr(base64.length() - 9) != "_MOCK_B64") {
            throw TCFSException(ErrorCode::InvalidArgument, "Invalid mock base64 format");
        }
        
        std::string hex_part = base64.substr(0, base64.length() - 9);
        return fromHex(hex_part);
    }
};

#endif

std::unique_ptr<CryptoProvider> createCryptoProvider() {
#if TCFS_HAS_OPENSSL
    return std::unique_ptr<CryptoProvider>(new OpenSSLCryptoProvider());
#else
    return std::unique_ptr<CryptoProvider>(new MockCryptoProvider());
#endif
}

} // namespace tcfs