#include "tcfs/Errors.hpp"

namespace tcfs {

std::string to_string(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS:
            return "Success";
        case ErrorCode::CRYPTO_ERROR:
            return "Cryptographic operation failed";
        case ErrorCode::CryptoInitFailed:
            return "Cryptographic initialization failed";
        case ErrorCode::EncryptionFailed:
            return "Encryption operation failed";
        case ErrorCode::DecryptionFailed:
            return "Decryption operation failed";
        case ErrorCode::InvalidKey:
            return "Invalid cryptographic key";
        case ErrorCode::InvalidIV:
            return "Invalid initialization vector";
        case ErrorCode::TIME_NOT_REACHED:
            return "Unlock time has not been reached";
        case ErrorCode::TimeNotReached:
            return "Unlock time has not been reached";
        case ErrorCode::InvalidTimeFormat:
            return "Invalid time format";
        case ErrorCode::ClockManipulation:
            return "Clock manipulation detected";
        case ErrorCode::FILE_NOT_FOUND:
            return "File not found";
        case ErrorCode::FILE_ACCESS_ERROR:
            return "File access error";
        case ErrorCode::FileNotFound:
            return "File not found";
        case ErrorCode::FileAccessDenied:
            return "File access denied";
        case ErrorCode::InvalidMetadata:
            return "Invalid metadata";
        case ErrorCode::CorruptedData:
            return "Data corruption detected";
        case ErrorCode::INVALID_POLICY:
            return "Invalid policy configuration";
        case ErrorCode::POLICY_VIOLATION:
            return "Policy violation";
        case ErrorCode::InvalidPolicy:
            return "Invalid policy configuration";
        case ErrorCode::PolicyViolation:
            return "Policy violation";
        case ErrorCode::AUDIT_LOG_ERROR:
            return "Audit log error";
        case ErrorCode::AuditLogCorrupted:
            return "Audit log corrupted";
        case ErrorCode::HashChainBroken:
            return "Hash chain integrity broken";
        case ErrorCode::INVALID_FORMAT:
            return "Invalid file format";
        case ErrorCode::UNKNOWN_ERROR:
            return "Unknown error";
        case ErrorCode::InvalidArgument:
            return "Invalid argument";
        case ErrorCode::InternalError:
            return "Internal error";
        case ErrorCode::NotImplemented:
            return "Feature not implemented";
        default:
            return "Unknown error";
    }
}

TCFSException::TCFSException(ErrorCode code, const std::string& message)
    : code_(code), message_(message) {
}

const char* TCFSException::what() const noexcept {
    if (what_message_.empty()) {
        what_message_ = to_string(code_);
        if (!message_.empty()) {
            what_message_ += ": " + message_;
        }
    }
    return what_message_.c_str();
}

} // namespace tcfs