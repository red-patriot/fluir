# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.25)

project(
    fluir
    DESCRIPTION "The Fluir data-flow programming language."
    LANGUAGES CXX
    VERSION 0.1.1
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(disallow-in-source-builds)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(
    FLUIR_BUILD_TESTS
    "Enable building tests and test infrastructure. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

option(
    FLUIR_EXPORT_COMPILE_COMMANDS
    "Create a compile_commands.json when building. Default: ${PROJECT_IS_TOP_LEVEL}. Values: { ON, OFF }."
    ${PROJECT_IS_TOP_LEVEL}
)

add_subdirectory(bytecode)
add_subdirectory(compiler)
add_subdirectory(vm)

if (FLUIR_BUILD_TESTS)
    enable_testing()
endif ()

include(install-fluir)

setup_fluir_install()
