# TCFS - Time Capsule File System

[![Build Status](https://github.com/code-alchemist01/chronovault/workflows/CI/badge.svg)](https://github.com/code-alchemist01/chronovault/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)

**TCFS (Time Capsule File System)** is a secure, time-based file encryption system that allows you to lock files until a specific future date and time. Think of it as a digital time capsule for your important documents, messages, or any files you want to access only after a certain period.

## 🌟 Features

- **⏰ Time-Based Access Control**: Lock files until a specific future date and time
- **🔐 Military-Grade Encryption**: AES-256-GCM encryption with PBKDF2 key derivation
- **🛡️ Secure by Design**: Original files are securely deleted after encryption
- **📝 Rich Metadata**: Store labels, notes, and policy information with encrypted files
- **🔍 Status Monitoring**: Check remaining time and file information without decryption
- **💻 Cross-Platform**: Works on Windows, Linux, and macOS
- **🎯 Simple CLI Interface**: Easy-to-use command-line interface

## 🚀 Quick Start

### Prerequisites

- **C++17** compatible compiler
- **CMake** 3.15 or higher
- **OpenSSL** development libraries
- **nlohmann/json** library

### Building from Source

```bash
# Clone the repository
git clone https://github.com/code-alchemist01/chronovault.git
cd chronovault

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# The executable will be in build/src/cli/Release/tcfs.exe (Windows)
# or build/src/cli/tcfs (Linux/macOS)
```

### Installation

After building, you can copy the executable to your PATH or use it directly from the build directory.

## 📖 Usage

### 1. Initialize a TCFS Store

First, create a TCFS store (a directory where encrypted files will be stored):

```bash
tcfs --store ./my_capsules init --owner "Your Name"
```

### 2. Lock a File (Create Time Capsule)

Encrypt and lock a file until a specific date:

```bash
tcfs --store ./my_capsules lock secret_document.txt \
  --unlock-at "2025-12-25T09:00:00Z" \
  --label "Christmas Message" \
  --notes "A special message for Christmas morning"
```

**Important**: The original file will be securely deleted after encryption!

### 3. Check File Status

View information about a locked file without decrypting it:

```bash
tcfs --store ./my_capsules status secret_document.txt.tcfs
```

Output example:
```
Status for: secret_document.txt.tcfs
Store file: "./my_capsules/secret_document.txt.tcfs"
Metadata file: ./my_capsules/secret_document.txt.tcfs.meta
Policy: Policy{unlock_at=2025-12-25T09:00:00Z, owner=Your Name, label=Christmas Message, algorithm=AES-256-GCM, kdf=pbkdf2}
Unlock time: 2025-12-25T09:00:00Z
Time remaining: 8640000 seconds
Can unlock: No
Created at: 2024-01-01T12:00:00Z
Original filename: secret_document.txt
Tool version: 0.1.0
```

### 4. Unlock a File

Attempt to decrypt and restore a file (only works if the unlock time has passed):

```bash
tcfs --store ./my_capsules unlock secret_document.txt.tcfs --output restored_document.txt
```

If the time hasn't arrived yet:
```
Cannot unlock yet. Time remaining: 8639950 seconds
Unlock time: 2025-12-25T09:00:00Z
```

If the time has arrived:
```
File unlocked successfully!
Decrypted file: restored_document.txt
```

<img width="688" height="563" alt="Ekran görüntüsü 2025-09-26 174052" src="https://github.com/user-attachments/assets/1b315dff-460d-488d-a675-f9f31f52e42a" />

## 🏗️ Architecture

### Core Components

1. **TCFS Store**: A directory containing encrypted files and metadata
2. **Encrypted Files** (`.tcfs`): AES-256-GCM encrypted file content
3. **Metadata Files** (`.tcfs.meta`): JSON files containing policy and file information
4. **Policy Engine**: Enforces time-based access control rules

### Security Features

- **AES-256-GCM Encryption**: Authenticated encryption providing both confidentiality and integrity
- **PBKDF2 Key Derivation**: Secure key derivation from passwords with configurable iterations
- **Time-Based Access Control**: Files cannot be decrypted before the specified unlock time
- **Secure File Deletion**: Original files are overwritten and deleted after encryption
- **Metadata Protection**: Critical policy information is stored separately and validated

### File Structure

```
my_capsules/                    # TCFS Store
├── config.json                 # Store configuration
├── document1.txt.tcfs          # Encrypted file
├── document1.txt.tcfs.meta     # Metadata and policy
├── photo.jpg.tcfs              # Another encrypted file
└── photo.jpg.tcfs.meta         # Its metadata
```

## 🔧 Configuration

### Store Configuration (`config.json`)

```json
{
  "version": "1.0",
  "owner": "Your Name",
  "kdf": "pbkdf2",
  "created_at": "2024-01-01T12:00:00Z"
}
```

### Metadata Structure (`.tcfs.meta`)

```json
{
  "version": "1.0",
  "policy": {
    "unlock_at": "2025-12-25T09:00:00Z",
    "owner": "Your Name",
    "label": "Christmas Message",
    "notes": "A special message for Christmas morning",
    "algorithm": "AES-256-GCM",
    "kdf": "pbkdf2"
  },
  "created_at": "2024-01-01T12:00:00Z",
  "original_filename": "secret_document.txt",
  "tool_version": "0.1.0"
}
```

<img width="580" height="483" alt="Ekran görüntüsü 2025-09-26 174108" src="https://github.com/user-attachments/assets/b36270ed-d6f6-4951-b9b6-400ab308601d" />


## 🛠️ Development

### Project Structure

```
tcfs/
├── src/
│   ├── libtcfs/           # Core library
│   │   ├── crypto/        # Cryptographic functions
│   │   ├── policy/        # Policy management
│   │   ├── store/         # Store operations
│   │   └── utils/         # Utility functions
│   └── cli/               # Command-line interface
├── include/tcfs/          # Public headers
├── tests/                 # Unit tests
├── examples/              # Example programs
└── cmake/                 # CMake modules
```

### Building with Debug Information

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

### Running Tests

```bash
# Build tests (Release configuration recommended)
cmake --build . --config Release

# Run tests
ctest -C Release --verbose  # All platforms

# Or run tests directly
./build/tests/Release/tcfs_tests  # Linux/macOS
# or
.\build\tests\Release\tcfs_tests.exe  # Windows
```

**Test Results**: All 32 unit tests pass successfully, including:
- ✅ Cryptographic operations (AES-256-GCM, PBKDF2)
- ✅ Policy management and validation
- ✅ Error handling and edge cases
- ✅ File operations and time-based access control

## 🔒 Security Considerations

### What TCFS Protects Against

- **Premature Access**: Files cannot be accessed before the specified time
- **Data Tampering**: AES-GCM provides authentication and integrity checking
- **Unauthorized Decryption**: Strong encryption with secure key derivation

### What TCFS Does NOT Protect Against

- **System Clock Manipulation**: If an attacker can modify the system clock, they might bypass time restrictions
- **Physical Access**: If an attacker has physical access to the system and can modify the binary
- **Side-Channel Attacks**: Advanced cryptographic attacks are outside the scope of this implementation
- **Quantum Computing**: Current encryption methods may be vulnerable to future quantum computers

### Best Practices

1. **Backup Encrypted Files**: Store copies in multiple secure locations
2. **Remember Unlock Times**: Keep a record of when files can be unlocked
3. **Secure Your System**: Ensure your system clock is accurate and secure
4. **Regular Updates**: Keep TCFS updated to the latest version

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guidelines](CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes and add tests
4. Ensure all tests pass: `cmake --build . --target test`
5. Commit your changes: `git commit -m 'Add amazing feature'`
6. Push to the branch: `git push origin feature/amazing-feature`
7. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **OpenSSL** for cryptographic functions
- **nlohmann/json** for JSON parsing
- **CLI11** for command-line parsing
- All contributors and users of TCFS

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/code-alchemist01/chronovault/issues)
- **Discussions**: [GitHub Discussions](https://github.com/code-alchemist01/chronovault/discussions)
- **Documentation**: [Wiki](https://github.com/code-alchemist01/chronovault/wiki)

## 🗺️ Update Ideas

- [ ] GUI App
- [ ] Mobile Apps (iOS/Android)
- [ ] Cloud Storage Integration
- [ ] Multi-User Support
- [ ] Advanced Policy Options
- [ ] Backup and Recovery Tools

---

**⚠️ Disclaimer**: TCFS is provided as-is without any warranty. Always backup important files before encryption. The developers are not responsible for any data loss.

---

*For Turkish documentation, see [README-TR.md](README-TR.md)*
