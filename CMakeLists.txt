cmake_minimum_required(VERSION 3.16)

# Project information
project(elib VERSION 1.1.2 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build tests (default to OFF)
option(ELIB_BUILD_TESTS "Build the test suite" OFF)

configure_file("cmake/version.h.in" "${PROJECT_SOURCE_DIR}/include/elib/version.h" @ONLY)

# Add subdirectories for library and tests
add_subdirectory(src)

if(ELIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation (optional)
install(FILES LICENSE_1_0.txt DESTINATION ${CMAKE_INSTALL_PREFIX})
