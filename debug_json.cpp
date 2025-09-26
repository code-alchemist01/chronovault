#include <iostream>
#include <chrono>
#include "tcfs/Policy.hpp"

using namespace tcfs;

int main() {
    try {
        // Create a policy similar to the test
        Policy policy;
        
        auto unlock_time = std::chrono::system_clock::now() + std::chrono::hours(24);
        policy.setUnlockTime(unlock_time);
        policy.setOwner("test_user");
        policy.setLabel("Test Capsule");
        policy.setNotes("Test notes");
        policy.setGracePeriodMinutes(30);
        policy.setAlgorithm(CryptoAlgorithm::AES_256_GCM);
        policy.setKDFType(KDFType::PBKDF2);
        
        std::cout << "Original policy created successfully" << std::endl;
        
        // Serialize to JSON
        auto json_data = policy.toJSON();
        std::cout << "JSON serialization successful" << std::endl;
        std::cout << "JSON: " << json_data << std::endl;
        
        // Create new policy and deserialize
        Policy new_policy;
        new_policy.fromJSON(json_data);
        std::cout << "JSON deserialization successful" << std::endl;
        
        // Check if fields match
        std::cout << "Original unlock time: " << policy.getUnlockTime().time_since_epoch().count() << std::endl;
        std::cout << "New unlock time: " << new_policy.getUnlockTime().time_since_epoch().count() << std::endl;
        std::cout << "Times match: " << (new_policy.getUnlockTime() == policy.getUnlockTime()) << std::endl;
        
        std::cout << "All tests passed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}