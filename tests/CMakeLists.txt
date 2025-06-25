include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  trompeloeil
  GIT_REPOSITORY https://github.com/rollbear/trompeloeil.git
  GIT_TAG        v47
)

FetchContent_MakeAvailable(trompeloeil)


add_executable(tests
  array.cpp
  aligned_storage.cpp
  timer.cpp
  stream.cpp
)

target_link_libraries(tests PRIVATE elib Catch2::Catch2WithMain trompeloeil::trompeloeil)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)