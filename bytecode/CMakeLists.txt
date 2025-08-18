add_library(fluir.code INTERFACE)

add_library(
        fluir::code
        ALIAS
        fluir.code
)

target_include_directories(
        fluir.code INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
        ${CMAKE_CURRENT_SOURCE_DIR}/include/bytecode/version.hpp)
