cmake_minimum_required(VERSION 3.25...4.3)
project(elib VERSION 2.6.0 LANGUAGES CXX)

add_library(elib STATIC)
add_library(elib::elib ALIAS elib)

add_subdirectory(src)

if(PROJECT_IS_TOP_LEVEL)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()
