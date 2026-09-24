cmake_minimum_required(VERSION 3.16)
project(option_pricer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Core pricing library: everything the demo app and tests link against.
add_library(pricer
    src/option.cpp
    src/black_scholes.cpp
    src/monte_carlo.cpp
    src/greeks.cpp
)
target_include_directories(pricer PUBLIC include)
target_compile_options(pricer PRIVATE -Wall -Wextra -Wpedantic)

add_executable(pricer_demo apps/main.cpp)
target_link_libraries(pricer_demo PRIVATE pricer)

add_executable(convergence apps/convergence.cpp)
target_link_libraries(convergence PRIVATE pricer)

add_executable(greeks apps/greeks.cpp)
target_link_libraries(greeks PRIVATE pricer)

# Unit tests (GoogleTest, downloaded at configure time).
option(PRICER_BUILD_TESTS "Build the unit tests" ON)
if(PRICER_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(pricer_tests
        tests/test_black_scholes.cpp
        tests/test_monte_carlo.cpp
        tests/test_greeks.cpp
    )
    target_link_libraries(pricer_tests PRIVATE pricer GTest::gtest_main)
    target_compile_options(pricer_tests PRIVATE -Wall -Wextra -Wpedantic)

    include(GoogleTest)
    gtest_discover_tests(pricer_tests)
endif()
