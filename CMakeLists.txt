cmake_minimum_required(VERSION 3.25)
project(
    fluir.compiler
    DESCRIPTION "The compiler for the Fluir data-flow programming language."
    LANGUAGES CXX
    VERSION 0.1.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(
    FLUIR_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

include(CTestUseLaunchers)
include(FetchContent)

add_subdirectory(src)

if(FLUIR_BUILD_TESTS)
    enable_testing()
    include(cmake/get-gtest.cmake)
    add_subdirectory(test)
endif()
