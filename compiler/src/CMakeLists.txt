add_library(fluir.libcompiler)
add_library(fluir::compiler ALIAS fluir.libcompiler)

target_sources(fluir.libcompiler PRIVATE dummy.cpp)

target_include_directories(fluir.libcompiler PUBLIC ${PROJECT_SOURCE_DIR})

add_executable(fluir.compiler)
target_sources(fluir.compiler PRIVATE main.cpp)

target_link_libraries(fluir.compiler PRIVATE fluir::compiler)
