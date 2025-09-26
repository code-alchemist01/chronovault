#include <CLI/CLI.hpp>
#include <tcfs/Policy.hpp>
#include <tcfs/CryptoProvider.hpp>
#include <tcfs/Errors.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

/**
 * @brief Simple CLI application for Time Capsule File System
 */
class TCFSApp {
public:
    TCFSApp() : crypto_(tcfs::createCryptoProvider()) {}
    
    int run(int argc, char** argv) {
        CLI::App app{"Time Capsule File System - Secure time-locked file encryption", "tcfs"};
        app.require_subcommand(1);
        
        // Global options
        app.add_option("--store", store_path_, "Path to TCFS store directory")
           ->default_val(get_default_store_path());
        
        // Subcommands
        setup_init_command(app);
        setup_lock_command(app);
        setup_unlock_command(app);
        setup_status_command(app);
        setup_list_command(app);
        
        try {
            app.parse(argc, argv);
            return 0;
        } catch (const CLI::ParseError& e) {
            return app.exit(e);
        } catch (const tcfs::TCFSException& e) {
            std::cerr << "TCFS Error: " << e.what() << std::endl;
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

private:
    std::unique_ptr<tcfs::CryptoProvider> crypto_;
    std::string store_path_;
    
    std::string get_default_store_path() {
        auto home = std::getenv("HOME");
        if (!home) {
            home = std::getenv("USERPROFILE"); // Windows
        }
        if (home) {
            return (fs::path(home) / ".tcfs").string();
        }
        return ".tcfs";
    }
    
    void setup_init_command(CLI::App& app) {
        auto init_cmd = app.add_subcommand("init", "Initialize TCFS store");
        
        auto owner = std::make_shared<std::string>();
        auto kdf = std::make_shared<std::string>("argon2id");
        
        init_cmd->add_option("--owner", *owner, "Owner email address")->required();
        init_cmd->add_option("--kdf", *kdf, "Key derivation function (pbkdf2|argon2id)")
                ->check(CLI::IsMember({"pbkdf2", "argon2id"}));
        
        init_cmd->callback([this, owner, kdf]() {
            cmd_init(*owner, *kdf);
        });
    }
    
    void setup_lock_command(CLI::App& app) {
        auto lock_cmd = app.add_subcommand("lock", "Lock a file in time capsule");
        
        auto input_file = std::make_shared<std::string>();
        auto output_file = std::make_shared<std::string>();
        auto unlock_at = std::make_shared<std::string>();
        auto label = std::make_shared<std::string>();
        auto notes = std::make_shared<std::string>();
        
        lock_cmd->add_option("input", *input_file, "Input file to lock")->required();
        lock_cmd->add_option("-o,--output", *output_file, "Output encrypted file");
        lock_cmd->add_option("--unlock-at", *unlock_at, "Unlock time (RFC3339 format)")->required();
        lock_cmd->add_option("--label", *label, "Label for the time capsule");
        lock_cmd->add_option("--notes", *notes, "Notes for the time capsule");
        
        lock_cmd->callback([this, input_file, output_file, unlock_at, label, notes]() {
            if (output_file->empty()) {
                *output_file = *input_file + ".tcfs";
            }
            cmd_lock(*input_file, *output_file, *unlock_at, *label, *notes);
        });
    }
    
    void setup_unlock_command(CLI::App& app) {
        auto unlock_cmd = app.add_subcommand("unlock", "Unlock a time capsule file");
        
        auto input_file = std::make_shared<std::string>();
        auto output_file = std::make_shared<std::string>();
        
        unlock_cmd->add_option("input", *input_file, "Encrypted file to unlock")->required();
        unlock_cmd->add_option("-o,--output", *output_file, "Output decrypted file")->required();
        
        unlock_cmd->callback([this, input_file, output_file]() {
            cmd_unlock(*input_file, *output_file);
        });
    }
    
    void setup_status_command(CLI::App& app) {
        auto status_cmd = app.add_subcommand("status", "Show status of a time capsule file");
        
        auto input_file = std::make_shared<std::string>();
        
        status_cmd->add_option("input", *input_file, "Encrypted file to check")->required();
        
        status_cmd->callback([this, input_file]() {
            cmd_status(*input_file);
        });
    }
    
    void setup_list_command(CLI::App& app) {
        auto list_cmd = app.add_subcommand("list", "List time capsule files in store");
        
        list_cmd->callback([this]() {
            cmd_list();
        });
    }
    
    void cmd_init(const std::string& owner, const std::string& kdf) {
        std::cout << "Initializing TCFS store at: " << store_path_ << std::endl;
        std::cout << "Owner: " << owner << std::endl;
        std::cout << "KDF: " << kdf << std::endl;
        
        // Create store directory
        fs::create_directories(store_path_);
        
        // Create config file
        nlohmann::json config;
        config["version"] = "0.1.0";
        config["owner"] = owner;
        config["kdf"] = kdf;
        config["created_at"] = tcfs::time_utils::format_rfc3339(tcfs::time_utils::now());
        
        auto config_path = fs::path(store_path_) / "config.json";
        std::ofstream config_file(config_path);
        if (!config_file) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FILE_ACCESS_ERROR, "Failed to create config file: " + config_path.string());
        }
        config_file << config.dump(2) << std::endl;
        
        std::cout << "TCFS store initialized successfully!" << std::endl;
    }
    
    void cmd_lock(const std::string& input_file, const std::string& output_file,
                  const std::string& unlock_at, const std::string& label, const std::string& notes) {
        
        std::cout << "Locking file: " << input_file << std::endl;
        std::cout << "Output: " << output_file << std::endl;
        std::cout << "Unlock at: " << unlock_at << std::endl;
        
        // Check if input file exists
        if (!fs::exists(input_file)) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FileNotFound, "Input file not found: " + input_file);
        }
        
        // Load owner from config if store exists
        std::string owner = "user@example.com"; // Default
        auto config_path = fs::path(store_path_) / "config.json";
        if (fs::exists(config_path)) {
            std::ifstream config_file(config_path, std::ios::binary);
            if (config_file) {
                try {
                    nlohmann::json config;
                    config_file >> config;
                    if (config.contains("owner")) {
                        owner = config["owner"];
                    }
                } catch (const nlohmann::json::exception& e) {
                    std::cerr << "Warning: Failed to parse config file: " << e.what() << std::endl;
                }
            }
        }
        
        // Create policy
        tcfs::Policy policy;
        policy.set_unlock_time(unlock_at);
        policy.set_owner(owner);
        policy.set_label(label);
        policy.set_notes(notes);
        
        auto validation = policy.validate();
        if (!validation) {
            throw tcfs::TCFSException(validation.error(), validation.error_message());
        }
        
        // Read input file
        std::ifstream file(input_file, std::ios::binary);
        if (!file) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FILE_ACCESS_ERROR, "Failed to read input file: " + input_file);
        }
        std::vector<uint8_t> file_data((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
        file.close();
        
        // Generate encryption materials
        auto data_key = crypto_->generateKey();
        auto iv = crypto_->generateIV();
        
        // Encrypt file data
        auto encrypted_data = crypto_->encrypt(file_data, data_key, iv);
        
        // Write encrypted file to store
        auto store_output_path = fs::path(store_path_) / (fs::path(input_file).filename().string() + ".tcfs");
        std::ofstream output(store_output_path, std::ios::binary);
        if (!output) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FILE_ACCESS_ERROR, "Failed to write encrypted file: " + store_output_path.string());
        }
        output.write(reinterpret_cast<const char*>(encrypted_data.ciphertext.data()), 
                    encrypted_data.ciphertext.size());
        output.close();
        
        // Create metadata file
        nlohmann::json metadata;
        metadata["policy"] = policy.to_json();
        metadata["iv"] = crypto_->toBase64(encrypted_data.iv);
        metadata["tag"] = crypto_->toBase64(encrypted_data.tag);
        metadata["data_key_encrypted"] = crypto_->toBase64(data_key.data); // Simple storage for now
        metadata["created_at"] = tcfs::time_utils::format_rfc3339(tcfs::time_utils::now());
        metadata["tool_version"] = "0.1.0";
        metadata["original_filename"] = fs::path(input_file).filename().string();
        
        auto metadata_path = store_output_path.string() + ".meta";
        std::ofstream meta_output(metadata_path, std::ios::binary);
        if (!meta_output) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FILE_ACCESS_ERROR, "Failed to write metadata file: " + metadata_path);
        }
        
        try {
            // Use dump with ensure_ascii=false to properly handle UTF-8
            std::string json_str = metadata.dump(2, ' ', false, nlohmann::json::error_handler_t::replace);
            meta_output << json_str << std::endl;
        } catch (const nlohmann::json::exception& e) {
            meta_output.close();
            throw tcfs::TCFSException(tcfs::ErrorCode::FILE_ACCESS_ERROR, "JSON serialization error: " + std::string(e.what()));
        }
        meta_output.close();
        
        // Delete original file (THIS IS THE KEY PART!)
        std::error_code ec;
        if (!fs::remove(input_file, ec)) {
            std::cerr << "Warning: Failed to delete original file: " << ec.message() << std::endl;
        }
        
        std::cout << "File locked successfully!" << std::endl;
        std::cout << "Encrypted file: " << store_output_path << std::endl;
        std::cout << "Metadata file: " << metadata_path << std::endl;
        std::cout << "Original file deleted for security!" << std::endl;
    }
    
    void cmd_unlock(const std::string& input_file, const std::string& output_file) {
        std::cout << "Attempting to unlock: " << input_file << std::endl;
        
        // Look for the file in store (either by name or with .tcfs extension)
        auto store_file_path = fs::path(store_path_) / (input_file + ".tcfs");
        auto metadata_path = store_file_path.string() + ".meta";
        
        // If not found, try with the input_file as is
        if (!fs::exists(store_file_path)) {
            store_file_path = fs::path(store_path_) / input_file;
            metadata_path = store_file_path.string() + ".meta";
        }
        
        // Check if encrypted file exists
        if (!fs::exists(store_file_path)) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FileNotFound, "Encrypted file not found in store: " + store_file_path.string());
        }
        
        // Check if metadata file exists
        if (!fs::exists(metadata_path)) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FileNotFound, "Metadata file not found: " + metadata_path);
        }
        
        // Read and parse metadata
        std::ifstream metadata_file(metadata_path, std::ios::binary);
        if (!metadata_file) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FILE_ACCESS_ERROR, "Failed to read metadata file: " + metadata_path);
        }

        nlohmann::json metadata;
        try {
            metadata_file >> metadata;
        } catch (const nlohmann::json::exception& e) {
            metadata_file.close();
            throw tcfs::TCFSException(tcfs::ErrorCode::InvalidMetadata, "JSON parsing error: " + std::string(e.what()));
        }
        metadata_file.close();
        
        // Parse policy from metadata
        if (!metadata.contains("policy")) {
            throw tcfs::TCFSException(tcfs::ErrorCode::InvalidMetadata, "Policy not found in metadata");
        }

        auto policy_result = tcfs::Policy::from_json(metadata["policy"], true); // Skip time validation for unlock
        if (!policy_result) {
            std::string error_msg = "Failed to parse policy from metadata";
            if (!policy_result.error_message().empty()) {
                error_msg += ": " + policy_result.error_message();
            }
            throw tcfs::TCFSException(tcfs::ErrorCode::InvalidMetadata, error_msg);
        }
        
        auto& policy = policy_result.value();
        
        // Check if unlock time has been reached
        if (!policy.is_unlock_time_reached()) {
            auto remaining = policy.time_remaining();
            std::cout << "Cannot unlock yet. Time remaining: " << remaining.count() << " seconds" << std::endl;
            std::cout << "Unlock time: " << policy.unlock_time_rfc3339() << std::endl;
            return;
        }
        
        std::cout << "Time check passed. Proceeding with decryption..." << std::endl;
        
        // Extract encryption parameters from metadata
        if (!metadata.contains("iv") || !metadata.contains("tag") || !metadata.contains("data_key_encrypted")) {
            throw tcfs::TCFSException(tcfs::ErrorCode::InvalidMetadata, "Missing encryption parameters in metadata");
        }
        
        auto iv = crypto_->fromBase64(metadata["iv"].get<std::string>());
        auto tag = crypto_->fromBase64(metadata["tag"].get<std::string>());
        auto data_key_bytes = crypto_->fromBase64(metadata["data_key_encrypted"].get<std::string>());
        tcfs::CryptoKey data_key(std::move(data_key_bytes));
        
        // Read encrypted file
        std::ifstream encrypted_file(store_file_path, std::ios::binary);
        if (!encrypted_file) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FILE_ACCESS_ERROR, "Failed to read encrypted file: " + store_file_path.string());
        }
        
        std::vector<uint8_t> encrypted_data((std::istreambuf_iterator<char>(encrypted_file)),
                                           std::istreambuf_iterator<char>());
        encrypted_file.close();
        
        // Prepare encrypted data structure
        tcfs::EncryptedData enc_data;
        enc_data.ciphertext = encrypted_data;
        enc_data.iv = iv;
        enc_data.tag = tag;
        
        // Decrypt the data
        auto decrypted_data = crypto_->decrypt(enc_data, data_key, iv);
        
        // Determine output file name
        std::string final_output = output_file;
        if (final_output.empty()) {
            // Use original filename from metadata if available
            if (metadata.contains("original_filename")) {
                final_output = metadata["original_filename"].get<std::string>();
            } else {
                // Fallback to input filename without .tcfs extension
                final_output = fs::path(input_file).stem().string();
            }
        }
        
        // Write decrypted file
        std::ofstream output(final_output, std::ios::binary);
        if (!output) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FILE_ACCESS_ERROR, "Failed to write decrypted file: " + final_output);
        }
        
        output.write(reinterpret_cast<const char*>(decrypted_data.data()), decrypted_data.size());
        output.close();
        
        std::cout << "File unlocked successfully!" << std::endl;
        std::cout << "Decrypted file: " << final_output << std::endl;
        std::cout << "Original encrypted file remains in store: " << store_file_path << std::endl;
    }
    
    void cmd_status(const std::string& input_file) {
        std::cout << "Status for: " << input_file << std::endl;
        
        // Look for the file in store (either by name or with .tcfs extension)
        auto store_file_path = fs::path(store_path_) / (input_file + ".tcfs");
        auto metadata_path = store_file_path.string() + ".meta";
        
        // If not found, try with the input_file as is
        if (!fs::exists(metadata_path)) {
            store_file_path = fs::path(store_path_) / input_file;
            metadata_path = store_file_path.string() + ".meta";
        }
        
        if (!fs::exists(metadata_path)) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FileNotFound, "Metadata file not found: " + metadata_path);
        }
        
        std::ifstream metadata_file(metadata_path, std::ios::binary);
        if (!metadata_file) {
            throw tcfs::TCFSException(tcfs::ErrorCode::FILE_ACCESS_ERROR, "Failed to read metadata file: " + metadata_path);
        }

        nlohmann::json metadata;
        try {
            metadata_file >> metadata;
        } catch (const nlohmann::json::exception& e) {
            metadata_file.close();
            throw tcfs::TCFSException(tcfs::ErrorCode::InvalidMetadata, "JSON parsing error: " + std::string(e.what()));
        }
        metadata_file.close();
        
        std::cout << "Store file: " << store_file_path << std::endl;
        std::cout << "Metadata file: " << metadata_path << std::endl;
        
        if (metadata.contains("policy")) {
            auto policy_result = tcfs::Policy::from_json(metadata["policy"]);
            if (policy_result) {
                auto& policy = policy_result.value();
                std::cout << "Policy: " << policy.to_string() << std::endl;
                std::cout << "Unlock time: " << policy.unlock_time_rfc3339() << std::endl;
                std::cout << "Time remaining: " << policy.time_remaining().count() << " seconds" << std::endl;
                std::cout << "Can unlock: " << (policy.is_unlock_time_reached() ? "Yes" : "No") << std::endl;
            } else {
                std::cout << "Warning: Failed to parse policy: " << policy_result.error_message() << std::endl;
                std::cout << "Raw policy data: " << metadata["policy"].dump() << std::endl;
            }
        } else {
            std::cout << "Warning: No policy found in metadata" << std::endl;
        }
        
        if (metadata.contains("created_at")) {
            std::cout << "Created at: " << metadata["created_at"].get<std::string>() << std::endl;
        }
        
        if (metadata.contains("original_filename")) {
            std::cout << "Original filename: " << metadata["original_filename"].get<std::string>() << std::endl;
        }
        
        if (metadata.contains("tool_version")) {
            std::cout << "Tool version: " << metadata["tool_version"].get<std::string>() << std::endl;
        }
    }
    
    void cmd_list() {
        std::cout << "Listing time capsules in store: " << store_path_ << std::endl;
        
        if (!fs::exists(store_path_)) {
            std::cout << "Store directory does not exist. Run 'tcfs init' first." << std::endl;
            return;
        }
        
        bool found_any = false;
        
        try {
            for (const auto& entry : fs::directory_iterator(store_path_)) {
                if (entry.is_regular_file()) {
                    auto path = entry.path();
                    
                    // Look for .tcfs files
                    if (path.extension() == ".tcfs") {
                        found_any = true;
                        auto metadata_path = path.string() + ".meta";
                        
                        std::cout << "\n=== " << path.filename().string() << " ===" << std::endl;
                        std::cout << "Encrypted file: " << path.string() << std::endl;
                        
                        // Try to read metadata if it exists
                        if (fs::exists(metadata_path)) {
                            try {
                                std::ifstream metadata_file(metadata_path, std::ios::binary);
                                nlohmann::json metadata;
                                metadata_file >> metadata;
                                
                                if (metadata.contains("original_filename")) {
                                    std::cout << "Original filename: " << metadata["original_filename"].get<std::string>() << std::endl;
                                }
                                
                                if (metadata.contains("created_at")) {
                                    std::cout << "Created at: " << metadata["created_at"].get<std::string>() << std::endl;
                                }
                                
                                if (metadata.contains("policy")) {
                                    auto policy_result = tcfs::Policy::from_json(metadata["policy"]);
                                    if (policy_result) {
                                        auto& policy = policy_result.value();
                                        std::cout << "Unlock time: " << policy.unlock_time_rfc3339() << std::endl;
                                        std::cout << "Can unlock: " << (policy.is_unlock_time_reached() ? "Yes" : "No") << std::endl;
                                        
                                        if (!policy.is_unlock_time_reached()) {
                                            auto remaining = policy.time_remaining();
                                            std::cout << "Time remaining: " << remaining.count() << " seconds" << std::endl;
                                        }
                                        
                                        if (!policy.label().empty()) {
                                            std::cout << "Label: " << policy.label() << std::endl;
                                        }
                                        
                                        if (!policy.notes().empty()) {
                                            std::cout << "Notes: " << policy.notes() << std::endl;
                                        }
                                    } else {
                                        std::cout << "Warning: Failed to parse policy: " << policy_result.error_message() << std::endl;
                                    }
                                } else {
                                    std::cout << "Warning: No policy found in metadata" << std::endl;
                                }
                            } catch (const std::exception& e) {
                                std::cout << "Warning: Could not read metadata: " << e.what() << std::endl;
                            }
                        } else {
                            std::cout << "Warning: Metadata file not found" << std::endl;
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Error reading store directory: " << e.what() << std::endl;
            return;
        }
        
        if (!found_any) {
            std::cout << "No time capsules found in store." << std::endl;
        }
    }
};

int main(int argc, char** argv) {
    TCFSApp app;
    return app.run(argc, argv);
}