# Tests CMakeLists.txt

# Find required packages for testing
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    # Download and build GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50f33f9142fd2db563d69b413611e5.zip
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Test files
set(TEST_SOURCES
    test_policy.cpp
    test_crypto.cpp
    test_errors.cpp
)

# Create test executable
add_executable(tcfs_tests ${TEST_SOURCES})

# Set target properties
set_target_properties(tcfs_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Link libraries
target_link_libraries(tcfs_tests
    PRIVATE
        libtcfs
        gtest_main
        gtest
        tcfs_dependencies
)

# Include directories
target_include_directories(tcfs_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Compiler warnings
if(TARGET tcfs_warnings)
    target_link_libraries(tcfs_tests PRIVATE tcfs_warnings)
endif()

# Enable testing
enable_testing()

# Add test
add_test(NAME tcfs_unit_tests COMMAND tcfs_tests)

# Test discovery for CTest
include(GoogleTest)
gtest_discover_tests(tcfs_tests)