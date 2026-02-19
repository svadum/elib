include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.12.0
)

FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  trompeloeil
  GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
  GIT_TAG        v49
)

FetchContent_MakeAvailable(trompeloeil)


add_executable(elib.test.unit
  host/elib.tweaks.h
  mock/assert.h
  mock/assert.cpp
  array.cpp
  aligned_storage.cpp
  timer.cpp
  stream.cpp
  circular_buffer.cpp
  external.cpp
  scope.cpp
  event_loop.cpp
  kernel.cpp
  list.cpp
)
add_executable(elib::test::unit ALIAS elib.test.unit)

set(HOST_TWEAKS_INCLUDE_PATH host)
target_include_directories(elib PUBLIC ${HOST_TWEAKS_INCLUDE_PATH})
target_link_libraries(elib.test.unit PRIVATE elib Catch2::Catch2WithMain trompeloeil::trompeloeil)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(elib.test.unit)