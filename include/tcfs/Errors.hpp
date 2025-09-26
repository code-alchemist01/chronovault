#pragma once

#include <string>
#include <system_error>

namespace tcfs {

/**
 * @brief Error codes for TCFS operations
 */
enum class ErrorCode {
    SUCCESS = 0,
    
    // Crypto errors
    CRYPTO_ERROR,
    CryptoInitFailed,
    EncryptionFailed,
    DecryptionFailed,
    InvalidKey,
    InvalidIV,
    
    // Time errors
    TIME_NOT_REACHED,
    TimeNotReached,
    InvalidTimeFormat,
    ClockManipulation,
    
    // File errors
    FILE_NOT_FOUND,
    FILE_ACCESS_ERROR,
    FileNotFound,
    FileAccessDenied,
    InvalidMetadata,
    CorruptedData,
    
    // Policy errors
    INVALID_POLICY,
    POLICY_VIOLATION,
    InvalidPolicy,
    PolicyViolation,
    
    // Audit errors
    AUDIT_LOG_ERROR,
    AuditLogCorrupted,
    HashChainBroken,
    
    // General errors
    INVALID_FORMAT,
    UNKNOWN_ERROR,
    InvalidArgument,
    InternalError,
    NotImplemented
};

/**
 * @brief Convert error code to string description
 */
std::string to_string(ErrorCode code);

/**
 * @brief TCFS exception class
 */
class TCFSException : public std::exception {
public:
    explicit TCFSException(ErrorCode code, const std::string& message = "");
    
    const char* what() const noexcept override;
    
    ErrorCode getErrorCode() const noexcept { return code_; }
    const std::string& getMessage() const noexcept { return message_; }

private:
    ErrorCode code_;
    std::string message_;
    mutable std::string what_message_;
};

/**
 * @brief Result type for operations that can fail
 */
template<typename T>
class Result {
public:
    // Success constructor
    explicit Result(T&& value) : has_value_(true), value_(std::move(value)) {}
    explicit Result(const T& value) : has_value_(true), value_(value) {}
    
    // Error constructor
    explicit Result(ErrorCode error) : has_value_(false), error_(error) {}
    Result(ErrorCode error, const std::string& message) 
        : has_value_(false), error_(error), error_message_(message) {}
    
    // Static factory methods
    static Result success(T&& value) { return Result(std::move(value)); }
    static Result success(const T& value) { return Result(value); }
    static Result error(ErrorCode code) { return Result(code); }
    static Result error(ErrorCode code, const std::string& message) { return Result(code, message); }
    
    // Destructor
    ~Result() {
        if (has_value_) {
            value_.~T();
        }
    }
    
    // Copy constructor
    Result(const Result& other) : has_value_(other.has_value_), error_message_(other.error_message_) {
        if (has_value_) {
            new(&value_) T(other.value_);
        } else {
            error_ = other.error_;
        }
    }
    
    // Move constructor
    Result(Result&& other) noexcept : has_value_(other.has_value_), error_message_(std::move(other.error_message_)) {
        if (has_value_) {
            new(&value_) T(std::move(other.value_));
        } else {
            error_ = other.error_;
        }
    }
    
    // Copy assignment
    Result& operator=(const Result& other) {
        if (this != &other) {
            if (has_value_) {
                value_.~T();
            }
            has_value_ = other.has_value_;
            error_message_ = other.error_message_;
            if (has_value_) {
                new(&value_) T(other.value_);
            } else {
                error_ = other.error_;
            }
        }
        return *this;
    }
    
    // Move assignment
    Result& operator=(Result&& other) noexcept {
        if (this != &other) {
            if (has_value_) {
                value_.~T();
            }
            has_value_ = other.has_value_;
            error_message_ = std::move(other.error_message_);
            if (has_value_) {
                new(&value_) T(std::move(other.value_));
            } else {
                error_ = other.error_;
            }
        }
        return *this;
    }
    
    // Check if result has value
    bool has_value() const noexcept { return has_value_; }
    explicit operator bool() const noexcept { return has_value_; }
    
    // Test API methods
    bool isSuccess() const noexcept { return has_value_; }
    bool isError() const noexcept { return !has_value_; }
    
    // Access value (throws if error)
    T& value() & {
        if (!has_value_) {
            throw TCFSException(error_, error_message_);
        }
        return value_;
    }
    
    const T& value() const & {
        if (!has_value_) {
            throw TCFSException(error_, error_message_);
        }
        return value_;
    }
    
    T&& value() && {
        if (!has_value_) {
            throw TCFSException(error_, error_message_);
        }
        return std::move(value_);
    }
    
    // Test API value access
    T& getValue() & { return value(); }
    const T& getValue() const & { return value(); }
    T&& getValue() && { return std::move(*this).value(); }
    
    // Access error
    ErrorCode error() const {
        if (has_value_) {
            throw std::logic_error("Result has value, not error");
        }
        return error_;
    }
    
    const std::string& error_message() const {
        if (has_value_) {
            throw std::logic_error("Result has value, not error");
        }
        return error_message_;
    }
    
    // Test API error access
    ErrorCode getErrorCode() const {
        return has_value_ ? ErrorCode::SUCCESS : error_;
    }
    
    const std::string& getErrorMessage() const {
        return error_message_;
    }

private:
    bool has_value_;
    union {
        T value_;
        ErrorCode error_;
    };
    std::string error_message_;
};

// Specialization for void
template<>
class Result<void> {
public:
    // Success constructor
    Result() : has_value_(true) {}
    
    // Error constructor
    explicit Result(ErrorCode error) : has_value_(false), error_(error) {}
    Result(ErrorCode error, const std::string& message) 
        : has_value_(false), error_(error), error_message_(message) {}
    
    // Static factory methods
    static Result success() { return Result(); }
    static Result error(ErrorCode code) { return Result(code); }
    static Result error(ErrorCode code, const std::string& message) { return Result(code, message); }
    
    bool has_value() const noexcept { return has_value_; }
    explicit operator bool() const noexcept { return has_value_; }
    
    // Test API methods
    bool isSuccess() const noexcept { return has_value_; }
    bool isError() const noexcept { return !has_value_; }
    
    void value() const {
        if (!has_value_) {
            throw TCFSException(error_, error_message_);
        }
    }
    
    ErrorCode error() const {
        if (has_value_) {
            throw std::logic_error("Result has value, not error");
        }
        return error_;
    }
    
    const std::string& error_message() const {
        if (has_value_) {
            throw std::logic_error("Result has value, not error");
        }
        return error_message_;
    }
    
    // Test API error access
    ErrorCode getErrorCode() const {
        return has_value_ ? ErrorCode::SUCCESS : error_;
    }
    
    const std::string& getErrorMessage() const {
        return error_message_;
    }

private:
    bool has_value_;
    ErrorCode error_;
    std::string error_message_;
};

} // namespace tcfs