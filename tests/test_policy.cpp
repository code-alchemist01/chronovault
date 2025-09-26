#include <gtest/gtest.h>
#include <tcfs/Policy.hpp>
#include <chrono>

using namespace tcfs;

class PolicyTest : public ::testing::Test {
protected:
    void SetUp() override {
        policy = std::make_unique<Policy>();
    }

    std::unique_ptr<Policy> policy;
};

TEST_F(PolicyTest, DefaultConstruction) {
    EXPECT_EQ(policy->getOwner(), "");
    EXPECT_EQ(policy->getLabel(), "");
    EXPECT_EQ(policy->getNotes(), "");
    EXPECT_EQ(policy->getGracePeriodMinutes(), 0);
    EXPECT_EQ(policy->getAlgorithm(), CryptoAlgorithm::AES_256_GCM);
    EXPECT_EQ(policy->getKDFType(), KDFType::PBKDF2);
}

TEST_F(PolicyTest, SetAndGetUnlockTime) {
    auto future_time = std::chrono::system_clock::now() + std::chrono::hours(24);
    policy->setUnlockTime(future_time);
    
    auto retrieved_time = policy->getUnlockTime();
    EXPECT_EQ(retrieved_time, future_time);
}

TEST_F(PolicyTest, IsUnlockTimeReached) {
    // Set unlock time to past
    auto past_time = std::chrono::system_clock::now() - std::chrono::hours(1);
    policy->setUnlockTime(past_time);
    EXPECT_TRUE(policy->isUnlockTimeReached());
    
    // Set unlock time to future
    auto future_time = std::chrono::system_clock::now() + std::chrono::hours(1);
    policy->setUnlockTime(future_time);
    EXPECT_FALSE(policy->isUnlockTimeReached());
}

TEST_F(PolicyTest, SetAndGetOwner) {
    std::string owner = "test_user";
    policy->setOwner(owner);
    EXPECT_EQ(policy->getOwner(), owner);
}

TEST_F(PolicyTest, SetAndGetLabel) {
    std::string label = "Test Capsule";
    policy->setLabel(label);
    EXPECT_EQ(policy->getLabel(), label);
}

TEST_F(PolicyTest, SetAndGetNotes) {
    std::string notes = "This is a test time capsule";
    policy->setNotes(notes);
    EXPECT_EQ(policy->getNotes(), notes);
}

TEST_F(PolicyTest, SetAndGetGracePeriod) {
    uint32_t grace_period = 30;
    policy->setGracePeriodMinutes(grace_period);
    EXPECT_EQ(policy->getGracePeriodMinutes(), grace_period);
}

TEST_F(PolicyTest, SetAndGetAlgorithm) {
    policy->setAlgorithm(CryptoAlgorithm::AES_256_GCM);
    EXPECT_EQ(policy->getAlgorithm(), CryptoAlgorithm::AES_256_GCM);
}

TEST_F(PolicyTest, SetAndGetKDFType) {
    policy->setKDFType(KDFType::PBKDF2);
    EXPECT_EQ(policy->getKDFType(), KDFType::PBKDF2);
}

TEST_F(PolicyTest, JSONSerialization) {
    // Set up policy
    auto unlock_time = std::chrono::system_clock::now() + std::chrono::hours(24);
    policy->setUnlockTime(unlock_time);
    policy->setOwner("test_user");
    policy->setLabel("Test Capsule");
    policy->setNotes("Test notes");
    policy->setGracePeriodMinutes(30);
    policy->setAlgorithm(CryptoAlgorithm::AES_256_GCM);
    policy->setKDFType(KDFType::PBKDF2);
    
    // Serialize to JSON
    auto json_data = policy->toJSON();
    
    // Create new policy and deserialize
    auto new_policy = std::make_unique<Policy>();
    new_policy->fromJSON(json_data);
    
    // Verify all fields match
    // Compare times with second precision (RFC3339 format loses sub-second precision)
    auto original_seconds = std::chrono::duration_cast<std::chrono::seconds>(
        policy->getUnlockTime().time_since_epoch()).count();
    auto deserialized_seconds = std::chrono::duration_cast<std::chrono::seconds>(
        new_policy->getUnlockTime().time_since_epoch()).count();
    EXPECT_EQ(deserialized_seconds, original_seconds);
    EXPECT_EQ(new_policy->getOwner(), policy->getOwner());
    EXPECT_EQ(new_policy->getLabel(), policy->getLabel());
    EXPECT_EQ(new_policy->getNotes(), policy->getNotes());
    EXPECT_EQ(new_policy->getGracePeriodMinutes(), policy->getGracePeriodMinutes());
    EXPECT_EQ(new_policy->getAlgorithm(), policy->getAlgorithm());
    EXPECT_EQ(new_policy->getKDFType(), policy->getKDFType());
}

TEST_F(PolicyTest, Validation) {
    // Valid policy should pass validation
    auto future_time = std::chrono::system_clock::now() + std::chrono::hours(1);
    policy->setUnlockTime(future_time);
    policy->setOwner("test_user");
    EXPECT_TRUE(policy->isValid());
    
    // Policy with past unlock time should fail validation
    auto past_time = std::chrono::system_clock::now() - std::chrono::hours(1);
    policy->setUnlockTime(past_time);
    EXPECT_FALSE(policy->isValid());
}