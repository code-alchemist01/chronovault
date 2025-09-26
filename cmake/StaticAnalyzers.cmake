# Static analysis tools configuration

option(ENABLE_CPPCHECK "Enable static analysis with cppcheck" OFF)
option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)

if(ENABLE_CPPCHECK)
    find_program(CPPCHECK cppcheck)
    if(CPPCHECK)
        set(CMAKE_CXX_CPPCHECK 
            ${CPPCHECK}
            --enable=warning,performance,portability,information,missingInclude
            --std=c++20
            --template="[{severity}][{id}] {message} {callstack} \(On {file}:{line}\)"
            --verbose
            --quiet
            --suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-suppress
        )
        message(STATUS "cppcheck found: ${CPPCHECK}")
    else()
        message(WARNING "cppcheck requested but not found")
    endif()
endif()

if(ENABLE_CLANG_TIDY)
    find_program(CLANGTIDY clang-tidy)
    if(CLANGTIDY)
        set(CMAKE_CXX_CLANG_TIDY 
            ${CLANGTIDY}
            -config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
        )
        message(STATUS "clang-tidy found: ${CLANGTIDY}")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()