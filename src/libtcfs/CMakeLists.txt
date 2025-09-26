# libtcfs library CMakeLists.txt

# Collect source files
set(LIBTCFS_SOURCES
    core/Errors.cpp
    core/Policy.cpp
    crypto/OpenSSLCryptoProvider.cpp
)

# Create the library
add_library(libtcfs STATIC ${LIBTCFS_SOURCES})

# Set target properties
set_target_properties(libtcfs PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME tcfs
)

# Include directories
target_include_directories(libtcfs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(libtcfs
    PUBLIC
        tcfs_dependencies
)

# Compiler warnings
if(TARGET tcfs_warnings)
    target_link_libraries(libtcfs PRIVATE tcfs_warnings)
endif()

# Install library
install(TARGETS libtcfs
    EXPORT tcfs-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)