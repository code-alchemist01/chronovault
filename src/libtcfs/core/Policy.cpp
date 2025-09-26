#include "tcfs/Policy.hpp"
#include <iomanip>
#include <sstream>
#include <regex>

namespace tcfs {

void Policy::set_unlock_time(const std::string& rfc3339_time) {
    auto result = time_utils::parse_rfc3339(rfc3339_time);
    if (!result) {
        throw TCFSException(result.error(), result.error_message());
    }
    unlock_at_ = result.value();
}

std::string Policy::unlock_time_rfc3339() const {
    return time_utils::format_rfc3339(unlock_at_);
}

bool Policy::is_unlock_time_reached() const {
    auto now = time_utils::now();
    auto grace_duration = std::chrono::seconds(grace_seconds_);
    return now >= (unlock_at_ - grace_duration);
}

std::chrono::seconds Policy::time_remaining() const {
    auto now = time_utils::now();
    auto grace_duration = std::chrono::seconds(grace_seconds_);
    auto effective_unlock_time = unlock_at_ - grace_duration;
    
    if (now >= effective_unlock_time) {
        return std::chrono::seconds(0);
    }
    
    return std::chrono::duration_cast<std::chrono::seconds>(effective_unlock_time - now);
}

Result<void> Policy::validate() const {
    if (owner_.empty()) {
        return Result<void>(ErrorCode::InvalidPolicy, "Owner cannot be empty");
    }
    
    if (unlock_at_ == TimePoint{}) {
        return Result<void>(ErrorCode::InvalidPolicy, "Unlock time must be set");
    }
    
    auto now = time_utils::now();
    if (unlock_at_ <= now) {
        return Result<void>(ErrorCode::InvalidPolicy, "Unlock time must be in the future");
    }
    
    return Result<void>();
}

nlohmann::json Policy::to_json() const {
    nlohmann::json json;
    json["unlock_at"] = unlock_time_rfc3339();
    json["owner"] = owner_;
    json["label"] = label_;
    json["notes"] = notes_;
    json["grace_seconds"] = grace_seconds_;
    json["algorithm"] = tcfs::to_string(algorithm_);
    json["kdf"] = tcfs::to_string(kdf_);
    return json;
}

Result<Policy> Policy::from_json(const nlohmann::json& json) {
    try {
        Policy policy;
        
        if (!json.contains("unlock_at") || !json["unlock_at"].is_string()) {
            return Result<Policy>(ErrorCode::InvalidPolicy, "Missing or invalid unlock_at field");
        }
        policy.set_unlock_time(json["unlock_at"].get<std::string>());
        
        if (json.contains("owner") && json["owner"].is_string()) {
            policy.set_owner(json["owner"].get<std::string>());
        }
        
        if (json.contains("label") && json["label"].is_string()) {
            policy.set_label(json["label"].get<std::string>());
        }
        
        if (json.contains("notes") && json["notes"].is_string()) {
            policy.set_notes(json["notes"].get<std::string>());
        }
        
        if (json.contains("grace_seconds") && json["grace_seconds"].is_number_unsigned()) {
            policy.set_grace_seconds(json["grace_seconds"].get<uint32_t>());
        }
        
        if (json.contains("algorithm") && json["algorithm"].is_string()) {
            auto algo_result = crypto_algorithm_from_string(json["algorithm"].get<std::string>());
            if (algo_result) {
                policy.set_algorithm(algo_result.value());
            }
        }
        
        if (json.contains("kdf") && json["kdf"].is_string()) {
            auto kdf_result = kdf_from_string(json["kdf"].get<std::string>());
            if (kdf_result) {
                policy.set_kdf(kdf_result.value());
            }
        }
        
        auto validation = policy.validate();
        if (!validation) {
            return Result<Policy>(validation.error(), validation.error_message());
        }
        
        return Result<Policy>(std::move(policy));
        
    } catch (const std::exception& e) {
        return Result<Policy>(ErrorCode::InvalidPolicy, std::string("JSON parsing error: ") + e.what());
    }
}

std::string Policy::to_string() const {
    std::ostringstream oss;
    oss << "Policy{";
    oss << "unlock_at=" << unlock_time_rfc3339();
    oss << ", owner=" << owner_;
    oss << ", label=" << label_;
    oss << ", algorithm=" << tcfs::to_string(algorithm_);
    oss << ", kdf=" << tcfs::to_string(kdf_);
    oss << "}";
    return oss.str();
}

namespace time_utils {

Result<Policy::TimePoint> parse_rfc3339(const std::string& timestamp) {
    // Simple RFC3339 parser for UTC timestamps ending with 'Z'
    // Format: YYYY-MM-DDTHH:MM:SSZ
    std::regex rfc3339_regex(R"((\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z)");
    std::smatch matches;
    
    if (!std::regex_match(timestamp, matches, rfc3339_regex)) {
        return Result<Policy::TimePoint>(ErrorCode::InvalidTimeFormat, "Invalid RFC3339 format");
    }
    
    try {
        int year = std::stoi(matches[1].str());
        int month = std::stoi(matches[2].str());
        int day = std::stoi(matches[3].str());
        int hour = std::stoi(matches[4].str());
        int minute = std::stoi(matches[5].str());
        int second = std::stoi(matches[6].str());
        
        // Basic validation
        if (month < 1 || month > 12 || day < 1 || day > 31 ||
            hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
            second < 0 || second > 59) {
            return Result<Policy::TimePoint>(ErrorCode::InvalidTimeFormat, "Invalid date/time values");
        }
        
        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        
        // Convert to time_t treating as UTC
        // We need to use timegm or equivalent since mktime assumes local time
        #ifdef _WIN32
        // Windows doesn't have timegm, so we use _mkgmtime
        auto time_t_val = _mkgmtime(&tm);
        #else
        auto time_t_val = timegm(&tm);
        #endif
        
        if (time_t_val == -1) {
            return Result<Policy::TimePoint>(ErrorCode::InvalidTimeFormat, "Failed to convert to time_t");
        }
        
        auto time_point = std::chrono::system_clock::from_time_t(time_t_val);
        
        return Result<Policy::TimePoint>(time_point);
        
    } catch (const std::exception& e) {
        return Result<Policy::TimePoint>(ErrorCode::InvalidTimeFormat, 
                                       std::string("Parsing error: ") + e.what());
    }
}

std::string format_rfc3339(const Policy::TimePoint& time) {
    auto time_t_val = std::chrono::system_clock::to_time_t(time);
    std::tm* utc_tm = std::gmtime(&time_t_val);
    
    std::ostringstream oss;
    oss << std::put_time(utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

Policy::TimePoint now() {
    return std::chrono::system_clock::now();
}

} // namespace time_utils

std::string to_string(CryptoAlgorithm algo) {
    switch (algo) {
        case CryptoAlgorithm::AES_256_GCM:
            return "AES-256-GCM";
        default:
            return "Unknown";
    }
}

std::string to_string(KDFType kdf) {
    switch (kdf) {
        case KDFType::PBKDF2:
            return "pbkdf2";
        case KDFType::Argon2id:
            return "argon2id";
        default:
            return "unknown";
    }
}

Result<CryptoAlgorithm> crypto_algorithm_from_string(const std::string& str) {
    if (str == "AES-256-GCM") {
        return Result<CryptoAlgorithm>(CryptoAlgorithm::AES_256_GCM);
    }
    return Result<CryptoAlgorithm>(ErrorCode::InvalidArgument, "Unknown crypto algorithm: " + str);
}

Result<KDFType> kdf_from_string(const std::string& str) {
    if (str == "pbkdf2") {
        return Result<KDFType>(KDFType::PBKDF2);
    }
    if (str == "argon2id") {
        return Result<KDFType>(KDFType::Argon2id);
    }
    return Result<KDFType>(ErrorCode::InvalidArgument, "Unknown KDF type: " + str);
}

} // namespace tcfs