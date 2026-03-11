include_guard(GLOBAL)

set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

# Inherit the Beman logic for Address Sanitizer if desired
if(BUILDSYS_SANITIZER STREQUAL "MaxSan")
    # /Zi flag is needed for debug symbols with ASAN 
    set(SANITIZER_FLAGS "/fsanitize=address /Zi")
endif()

# Set standard MSVC flags 
set(CMAKE_CXX_FLAGS_DEBUG_INIT "/EHsc /permissive- /MP ${SANITIZER_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG_INIT "/EHsc /permissive- /MP ${SANITIZER_FLAGS}")

set(RELEASE_FLAGS "/EHsc /permissive- /O2 /MP ${SANITIZER_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${RELEASE_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE_INIT "${RELEASE_FLAGS}")