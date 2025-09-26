# Find required packages
find_package(Threads REQUIRED)

# Try to find OpenSSL (optional for now)
find_package(OpenSSL QUIET)

# Try to find nlohmann_json
find_package(nlohmann_json QUIET)

if(NOT nlohmann_json_FOUND)
    # Download nlohmann_json if not found
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Create interface library for dependencies
add_library(tcfs_dependencies INTERFACE)

target_link_libraries(tcfs_dependencies
    INTERFACE
        Threads::Threads
        nlohmann_json::nlohmann_json
)

# Add OpenSSL if found
if(OpenSSL_FOUND)
    target_link_libraries(tcfs_dependencies
        INTERFACE
            OpenSSL::SSL
            OpenSSL::Crypto
    )
    target_compile_definitions(tcfs_dependencies
        INTERFACE
            TCFS_HAS_OPENSSL=1
    )
    message(STATUS "OpenSSL found and enabled")
else()
    message(WARNING "OpenSSL not found - cryptographic features will be limited")
    target_compile_definitions(tcfs_dependencies
        INTERFACE
            TCFS_HAS_OPENSSL=0
    )
endif()