add_library(fluir.libcompiler)
add_library(
    fluir::compiler
    ALIAS
    fluir.libcompiler
)

target_sources(fluir.libcompiler PRIVATE utility/diagnostics.cpp)

target_include_directories(
    fluir.libcompiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

add_executable(fluir.compiler main.cpp)

target_link_libraries(fluir.compiler PRIVATE fluir::compiler)
