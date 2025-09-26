cmake_minimum_required(VERSION 3.22)

project(TCFS 
    VERSION 0.1.0
    DESCRIPTION "Time Capsule File System - Secure time-locked file encryption"
    LANGUAGES CXX
)

# C++20 standard requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Compiler warnings and flags
include(cmake/CompilerWarnings.cmake)
include(cmake/StaticAnalyzers.cmake)

# Dependencies
include(cmake/Dependencies.cmake)

# Options
option(TCFS_BUILD_TESTS "Build tests" ON)
option(TCFS_BUILD_EXAMPLES "Build examples" ON)
option(TCFS_ENABLE_STATIC_ANALYSIS "Enable static analysis tools" OFF)

# Global include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add subdirectories
add_subdirectory(src)

if(TCFS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples directory is optional
if(TCFS_BUILD_EXAMPLES AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt")
    add_subdirectory(examples)
endif()

# Install configuration
include(GNUInstallDirs)
install(TARGETS tcfs_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)