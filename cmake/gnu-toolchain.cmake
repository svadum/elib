include_guard(GLOBAL)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

if(BUILDSYS_SANITIZER STREQUAL "MaxSan")
    # GCC Address and Undefined Behavior sanitizers
    set(SANITIZER_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")
endif()

# Global GCC flags
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -O0 -Wall -Wextra ${SANITIZER_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG_INIT "-g -O0 -Wall -Wextra ${SANITIZER_FLAGS}")

set(RELEASE_FLAGS "-O3 -DNDEBUG -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${RELEASE_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE_INIT "${RELEASE_FLAGS}")

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_LIST_DIR}")