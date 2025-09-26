#pragma once

#include "Errors.hpp"
#include <chrono>
#include <string>
#include <nlohmann/json.hpp>

namespace tcfs {

/**
 * @brief Cryptographic algorithm types
 */
enum class CryptoAlgorithm {
    AES_256_GCM
};

/**
 * @brief Key derivation function types
 */
enum class KDFType {
    PBKDF2,
    Argon2id
};

/**
 * @brief Time capsule policy configuration
 */
class Policy {
public:
    using TimePoint = std::chrono::system_clock::time_point;
    
    Policy() = default;
    
    // Setters
    void set_unlock_time(const TimePoint& time) { unlock_at_ = time; }
    void set_unlock_time(const std::string& rfc3339_time);
    void set_owner(const std::string& owner) { owner_ = owner; }
    void set_label(const std::string& label) { label_ = label; }
    void set_notes(const std::string& notes) { notes_ = notes; }
    void set_grace_seconds(uint32_t seconds) { grace_seconds_ = seconds; }
    void set_algorithm(CryptoAlgorithm algo) { algorithm_ = algo; }
    void set_kdf(KDFType kdf) { kdf_ = kdf; }
    
    // Getters
    const TimePoint& unlock_time() const { return unlock_at_; }
    const std::string& owner() const { return owner_; }
    const std::string& label() const { return label_; }
    const std::string& notes() const { return notes_; }
    uint32_t grace_seconds() const { return grace_seconds_; }
    CryptoAlgorithm algorithm() const { return algorithm_; }
    KDFType kdf() const { return kdf_; }
    
    // Test API compatibility methods
    void setUnlockTime(const TimePoint& time) { set_unlock_time(time); }
    void setOwner(const std::string& owner) { set_owner(owner); }
    void setLabel(const std::string& label) { set_label(label); }
    void setNotes(const std::string& notes) { set_notes(notes); }
    void setGracePeriodMinutes(uint32_t minutes) { grace_seconds_ = minutes * 60; }
    void setAlgorithm(CryptoAlgorithm algo) { set_algorithm(algo); }
    void setKDFType(KDFType kdf) { set_kdf(kdf); }
    
    const TimePoint& getUnlockTime() const { return unlock_time(); }
    const std::string& getOwner() const { return owner(); }
    const std::string& getLabel() const { return label(); }
    const std::string& getNotes() const { return notes(); }
    uint32_t getGracePeriodMinutes() const { return grace_seconds_ / 60; }
    CryptoAlgorithm getAlgorithm() const { return algorithm(); }
    KDFType getKDFType() const { return kdf(); }
    
    // Time utilities
    std::string unlock_time_rfc3339() const;
    bool is_unlock_time_reached() const;
    bool isUnlockTimeReached() const { return is_unlock_time_reached(); }
    std::chrono::seconds time_remaining() const;
    
    // Validation
    Result<void> validate() const;
    bool isValid() const { 
        auto result = validate(); 
        return result.has_value(); 
    }
    
    // Serialization
    nlohmann::json to_json() const;
    static Result<Policy> from_json(const nlohmann::json& json);
    
    // Test API compatibility methods for JSON
    std::string toJSON() const { return to_json().dump(); }
    void fromJSON(const std::string& json_str) { 
        auto json = nlohmann::json::parse(json_str);
        auto result = from_json(json);
        if (result.has_value()) {
            *this = result.value();
        } else {
            throw TCFSException(result.error(), result.error_message());
        }
    }
    
    // String conversion
    std::string to_string() const;

private:
    TimePoint unlock_at_;
    std::string owner_;
    std::string label_;
    std::string notes_;
    uint32_t grace_seconds_ = 0;
    CryptoAlgorithm algorithm_ = CryptoAlgorithm::AES_256_GCM;
    KDFType kdf_ = KDFType::PBKDF2;
};

/**
 * @brief Utility functions for time handling
 */
namespace time_utils {
    /**
     * @brief Parse RFC3339 timestamp to time_point
     */
    Result<Policy::TimePoint> parse_rfc3339(const std::string& timestamp);
    
    /**
     * @brief Format time_point to RFC3339 string
     */
    std::string format_rfc3339(const Policy::TimePoint& time);
    
    /**
     * @brief Get current system time
     */
    Policy::TimePoint now();
}

/**
 * @brief Convert enum values to/from strings
 */
std::string to_string(CryptoAlgorithm algo);
std::string to_string(KDFType kdf);
Result<CryptoAlgorithm> crypto_algorithm_from_string(const std::string& str);
Result<KDFType> kdf_from_string(const std::string& str);

} // namespace tcfs