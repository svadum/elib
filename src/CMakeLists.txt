set(ELIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(ELIB_IMPL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/elib")

# Add the library target
add_library(elib STATIC
    ${ELIB_IMPL_INCLUDE_DIR}/array.h
    ${ELIB_IMPL_INCLUDE_DIR}/callback.h
    ${ELIB_IMPL_INCLUDE_DIR}/circular_buffer.h
    ${ELIB_IMPL_INCLUDE_DIR}/expected.h
    ${ELIB_IMPL_INCLUDE_DIR}/span.h
    ${ELIB_IMPL_INCLUDE_DIR}/memory.h
    ${ELIB_IMPL_INCLUDE_DIR}/config.h
    ${ELIB_IMPL_INCLUDE_DIR}/stream.h
    ${ELIB_IMPL_INCLUDE_DIR}/utility.h
    ${ELIB_IMPL_INCLUDE_DIR}/version.h
    ${ELIB_IMPL_INCLUDE_DIR}/time/core_clock.h
    ${ELIB_IMPL_INCLUDE_DIR}/time/system_clock.h
    ${ELIB_IMPL_INCLUDE_DIR}/time/timer.h
    ${ELIB_IMPL_INCLUDE_DIR}/scope.h
    ${ELIB_IMPL_INCLUDE_DIR}/event_loop.h
    ${ELIB_IMPL_INCLUDE_DIR}/assert.h
    ${ELIB_IMPL_INCLUDE_DIR}/kernel.h
    ${ELIB_IMPL_INCLUDE_DIR}/task.h

    time/system_clock.cpp
    time/timer.cpp
    kernel.cpp
    task.cpp
)

target_include_directories(elib PUBLIC ${ELIB_INCLUDE_DIR})
target_compile_features(elib PUBLIC cxx_std_17 c_std_11)
set_target_properties(elib PROPERTIES
    CXX_STANDARD_REQUIRED ON
    C_STANDARD_REQUIRED   ON
    CXX_EXTENSIONS        OFF # Use -std=c++17 instead of -std=gnu++17
)

# Set compiler options
if(MSVC)
    target_compile_options(elib PRIVATE
        /W4 /WX /permissive- /diagnostics:classic
    )
elseif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(elib PRIVATE
        -Wall -Wextra -Wpedantic -Werror -Wshadow
        -Wuninitialized -Wundef -Wconversion -Wsign-conversion
        -Wunused -Wdouble-promotion
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(elib PRIVATE
            -Weverything -Wno-c++98-compat
        )
    endif()
endif()
