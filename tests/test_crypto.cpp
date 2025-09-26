#include <gtest/gtest.h>
#include <tcfs/CryptoProvider.hpp>
#include <memory>

using namespace tcfs;

class CryptoTest : public ::testing::Test {
protected:
    void SetUp() override {
        crypto = createCryptoProvider();
    }

    std::unique_ptr<CryptoProvider> crypto;
};

TEST_F(CryptoTest, KeyGeneration) {
    auto key = crypto->generateKey();
    EXPECT_EQ(key.data.size(), 32); // AES-256 key size
    EXPECT_FALSE(key.data.empty());
}

TEST_F(CryptoTest, IVGeneration) {
    auto iv = crypto->generateIV();
    EXPECT_EQ(iv.size(), 12); // GCM IV size
    EXPECT_FALSE(iv.empty());
    
    // Generate another IV and ensure they're different
    auto iv2 = crypto->generateIV();
    EXPECT_NE(iv, iv2);
}

TEST_F(CryptoTest, SaltGeneration) {
    auto salt = crypto->generateSalt();
    EXPECT_EQ(salt.size(), 16); // Standard salt size
    EXPECT_FALSE(salt.empty());
    
    // Generate another salt and ensure they're different
    auto salt2 = crypto->generateSalt();
    EXPECT_NE(salt, salt2);
}

TEST_F(CryptoTest, KeyDerivation) {
    std::string password = "test_password";
    auto salt = crypto->generateSalt();
    
    KDFParams params;
    params.iterations = 100000;
    params.memory_kb = 0; // Not used for PBKDF2
    params.parallelism = 0; // Not used for PBKDF2
    
    auto derived_key = crypto->deriveKey(password, salt, params);
    EXPECT_EQ(derived_key.data.size(), 32); // AES-256 key size
    EXPECT_FALSE(derived_key.data.empty());
    
    // Derive the same key again with same parameters
    auto derived_key2 = crypto->deriveKey(password, salt, params);
    EXPECT_EQ(derived_key.data, derived_key2.data);
    
    // Derive with different salt should produce different key
    auto salt2 = crypto->generateSalt();
    auto derived_key3 = crypto->deriveKey(password, salt2, params);
    EXPECT_NE(derived_key.data, derived_key3.data);
}

TEST_F(CryptoTest, EncryptionDecryption) {
    std::string plaintext = "Hello, Time Capsule!";
    auto key = crypto->generateKey();
    auto iv = crypto->generateIV();
    
    // Encrypt
    auto encrypted = crypto->encrypt(
        std::vector<uint8_t>(plaintext.begin(), plaintext.end()),
        key,
        iv
    );
    
    EXPECT_FALSE(encrypted.ciphertext.empty());
    EXPECT_FALSE(encrypted.tag.empty());
    EXPECT_EQ(encrypted.tag.size(), 16); // GCM tag size
    
    // Decrypt
    auto decrypted = crypto->decrypt(encrypted, key, iv);
    std::string decrypted_text(decrypted.begin(), decrypted.end());
    
    EXPECT_EQ(decrypted_text, plaintext);
}

TEST_F(CryptoTest, EncryptionWithWrongKey) {
    std::string plaintext = "Hello, Time Capsule!";
    auto key1 = crypto->generateKey();
    auto key2 = crypto->generateKey();
    auto iv = crypto->generateIV();
    
    // Encrypt with key1
    auto encrypted = crypto->encrypt(
        std::vector<uint8_t>(plaintext.begin(), plaintext.end()),
        key1,
        iv
    );
    
    // Try to decrypt with key2 - should throw exception
    EXPECT_THROW(crypto->decrypt(encrypted, key2, iv), TCFSException);
}

TEST_F(CryptoTest, SHA256Hashing) {
    std::string input = "Hello, World!";
    std::vector<uint8_t> data(input.begin(), input.end());
    
    auto hash = crypto->sha256(data);
    EXPECT_EQ(hash.size(), 32); // SHA256 hash size
    EXPECT_FALSE(hash.empty());
    
    // Hash the same data again
    auto hash2 = crypto->sha256(data);
    EXPECT_EQ(hash, hash2);
    
    // Hash different data
    std::string input2 = "Hello, World!!";
    std::vector<uint8_t> data2(input2.begin(), input2.end());
    auto hash3 = crypto->sha256(data2);
    EXPECT_NE(hash, hash3);
}

TEST_F(CryptoTest, HexConversion) {
    std::vector<uint8_t> data = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    
    auto hex_string = crypto->toHex(data);
    EXPECT_EQ(hex_string, "0123456789ABCDEF");
    
    auto converted_back = crypto->fromHex(hex_string);
    EXPECT_EQ(data, converted_back);
}

TEST_F(CryptoTest, Base64Conversion) {
    std::vector<uint8_t> data = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    
    auto base64_string = crypto->toBase64(data);
    EXPECT_FALSE(base64_string.empty());
    
    auto converted_back = crypto->fromBase64(base64_string);
    EXPECT_EQ(data, converted_back);
}

TEST_F(CryptoTest, EmptyDataHandling) {
    std::vector<uint8_t> empty_data;
    
    // Empty data should produce empty results for conversions
    auto hex_string = crypto->toHex(empty_data);
    EXPECT_TRUE(hex_string.empty());
    
    auto base64_string = crypto->toBase64(empty_data);
    EXPECT_TRUE(base64_string.empty());
    
    // Empty data should still produce valid hash
    auto hash = crypto->sha256(empty_data);
    EXPECT_EQ(hash.size(), 32);
}