# CLI application CMakeLists.txt

# Create the executable
add_executable(tcfs_cli main.cpp)

# Set target properties
set_target_properties(tcfs_cli PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME tcfs
)

# Link libraries
target_link_libraries(tcfs_cli
    PRIVATE
        libtcfs
        tcfs_dependencies
)

# Include directories
target_include_directories(tcfs_cli
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Compiler warnings
if(TARGET tcfs_warnings)
    target_link_libraries(tcfs_cli PRIVATE tcfs_warnings)
endif()

# Install executable
install(TARGETS tcfs_cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)