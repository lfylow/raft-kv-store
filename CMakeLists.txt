cmake_minimum_required(VERSION 3.16)
project(raftkv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)

# ---- Core library (networking + state machine) ----
add_library(raftkv_core
    src/net/socket.cpp
    src/kv/store.cpp
)
target_include_directories(raftkv_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(raftkv_core PUBLIC Threads::Threads)

# ---- Executables ----
add_executable(raftkv_server src/server_main.cpp)
target_link_libraries(raftkv_server PRIVATE raftkv_core)

add_executable(raftkv_client src/client_main.cpp)
target_link_libraries(raftkv_client PRIVATE raftkv_core)

# ---- Tests (GoogleTest via FetchContent) ----
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(raftkv_tests tests/kv_store_test.cpp)
target_link_libraries(raftkv_tests PRIVATE raftkv_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(raftkv_tests)
