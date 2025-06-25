set(ELIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(ELIB_IMPL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/elib")

# Add the library target
add_library(elib STATIC
    ${ELIB_IMPL_INCLUDE_DIR}/array.h
    ${ELIB_IMPL_INCLUDE_DIR}/callback.h
    ${ELIB_IMPL_INCLUDE_DIR}/circular_buffer.h
    ${ELIB_IMPL_INCLUDE_DIR}/span.h
    ${ELIB_IMPL_INCLUDE_DIR}/memory.h
    ${ELIB_IMPL_INCLUDE_DIR}/config.h
    ${ELIB_IMPL_INCLUDE_DIR}/stream.h
    ${ELIB_IMPL_INCLUDE_DIR}/version.h
    ${ELIB_IMPL_INCLUDE_DIR}/time/core_clock.h
    ${ELIB_IMPL_INCLUDE_DIR}/time/system_clock.h
    ${ELIB_IMPL_INCLUDE_DIR}/time/timer.h

    time/system_clock.cpp
    time/timer.cpp
)

target_include_directories(elib PUBLIC ${ELIB_INCLUDE_DIR})
target_compile_features(elib PUBLIC cxx_std_17)
set_target_properties(elib PROPERTIES CXX_EXTENSIONS OFF)

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

# # Installation rules for the library
# install(TARGETS elib
#     EXPORT elib-targets
#     LIBRARY DESTINATION lib
#     ARCHIVE DESTINATION lib
#     INCLUDES DESTINATION include
# )

# install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../include/elib/ DESTINATION include/elib)

# # Package configuration (optional)
# install(EXPORT elib-targets
#     FILE elibConfig.cmake
#     DESTINATION lib/cmake/elib
# )
