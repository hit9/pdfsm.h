cmake_minimum_required(VERSION 3.10)

project(pdfsm_tests)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

include_directories(".." ".")

find_package(Catch2 CONFIG REQUIRED)

# Targets
file(GLOB TEST_SOURCES *_test.cc)
add_executable(pdfsm_tests ${TEST_SOURCES})

target_link_libraries(pdfsm_tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(pdfsm_tests)
