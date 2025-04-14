find_package(tinyxml2 REQUIRED)
find_package(fmt REQUIRED)

add_library(fluir.libcompiler)
add_library(fluir::compiler ALIAS fluir.libcompiler)

set(FLUIR_AFG_SOURCES
    "afg/constant.cpp"
    "afg/flow_graph.cpp"
    "afg/operation.cpp"
)

target_sources(
    fluir.libcompiler
    PRIVATE ${FLUIR_AFG_SOURCES} afg_builder.cpp diagnostic.cpp parser.cpp
)

target_include_directories(
    fluir.libcompiler
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(fluir.libcompiler PUBLIC tinyxml2::tinyxml2 fmt::fmt)

add_executable(fluir.compiler)
target_sources(fluir.compiler PRIVATE main.cpp)

target_link_libraries(fluir.compiler PRIVATE fluir::compiler)
