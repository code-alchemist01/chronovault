#include <gtest/gtest.h>
#include <tcfs/Errors.hpp>

using namespace tcfs;

TEST(ErrorsTest, ErrorCodeToString) {
    EXPECT_EQ(to_string(ErrorCode::SUCCESS), "Success");
    EXPECT_EQ(to_string(ErrorCode::CRYPTO_ERROR), "Cryptographic operation failed");
    EXPECT_EQ(to_string(ErrorCode::TIME_NOT_REACHED), "Unlock time has not been reached");
    EXPECT_EQ(to_string(ErrorCode::FILE_NOT_FOUND), "File not found");
    EXPECT_EQ(to_string(ErrorCode::FILE_ACCESS_ERROR), "File access error");
    EXPECT_EQ(to_string(ErrorCode::INVALID_POLICY), "Invalid policy configuration");
    EXPECT_EQ(to_string(ErrorCode::POLICY_VIOLATION), "Policy violation");
    EXPECT_EQ(to_string(ErrorCode::AUDIT_LOG_ERROR), "Audit log error");
    EXPECT_EQ(to_string(ErrorCode::INVALID_FORMAT), "Invalid file format");
    EXPECT_EQ(to_string(ErrorCode::UNKNOWN_ERROR), "Unknown error");
}

TEST(ErrorsTest, TCFSExceptionConstruction) {
    ErrorCode code = ErrorCode::CRYPTO_ERROR;
    std::string message = "Test crypto error";
    
    TCFSException ex(code, message);
    
    EXPECT_EQ(ex.getErrorCode(), code);
    EXPECT_EQ(ex.getMessage(), message);
    
    std::string what_msg = ex.what();
    EXPECT_TRUE(what_msg.find("Cryptographic operation failed") != std::string::npos);
    EXPECT_TRUE(what_msg.find(message) != std::string::npos);
}

TEST(ErrorsTest, TCFSExceptionWithoutMessage) {
    ErrorCode code = ErrorCode::FILE_NOT_FOUND;
    
    TCFSException ex(code);
    
    EXPECT_EQ(ex.getErrorCode(), code);
    EXPECT_TRUE(ex.getMessage().empty());
    
    std::string what_msg = ex.what();
    EXPECT_TRUE(what_msg.find("File not found") != std::string::npos);
}

TEST(ErrorsTest, ResultSuccess) {
    Result<int> result = Result<int>::success(42);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
    EXPECT_EQ(result.getValue(), 42);
    EXPECT_EQ(result.getErrorCode(), ErrorCode::SUCCESS);
}

TEST(ErrorsTest, ResultError) {
    Result<int> result = Result<int>::error(ErrorCode::CRYPTO_ERROR, "Test error");
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.getErrorCode(), ErrorCode::CRYPTO_ERROR);
    EXPECT_EQ(result.getErrorMessage(), "Test error");
    
    // Accessing value on error should throw
    EXPECT_THROW(result.getValue(), TCFSException);
}

TEST(ErrorsTest, ResultVoidSuccess) {
    Result<void> result = Result<void>::success();
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
    EXPECT_EQ(result.getErrorCode(), ErrorCode::SUCCESS);
}

TEST(ErrorsTest, ResultVoidError) {
    Result<void> result = Result<void>::error(ErrorCode::FILE_ACCESS_ERROR, "Access denied");
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.getErrorCode(), ErrorCode::FILE_ACCESS_ERROR);
    EXPECT_EQ(result.getErrorMessage(), "Access denied");
}

TEST(ErrorsTest, ResultMoveSemantics) {
    // Test that Result can handle move-only types
    auto create_unique_ptr = []() -> Result<std::unique_ptr<int>> {
        return Result<std::unique_ptr<int>>::success(std::make_unique<int>(42));
    };
    
    auto result = create_unique_ptr();
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(*result.getValue(), 42);
}

TEST(ErrorsTest, ResultCopySemantics) {
    Result<std::string> original = Result<std::string>::success("Hello");
    Result<std::string> copy = original;
    
    EXPECT_TRUE(copy.isSuccess());
    EXPECT_EQ(copy.getValue(), "Hello");
    
    // Original should still be valid
    EXPECT_TRUE(original.isSuccess());
    EXPECT_EQ(original.getValue(), "Hello");
}

TEST(ErrorsTest, ResultErrorWithoutMessage) {
    Result<int> result = Result<int>::error(ErrorCode::UNKNOWN_ERROR);
    
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.getErrorCode(), ErrorCode::UNKNOWN_ERROR);
    EXPECT_TRUE(result.getErrorMessage().empty());
}