find_package(fmt REQUIRED)
find_package(tinyxml2 REQUIRED)

include(strict-warnings)

add_library(fluir.libcompiler)
add_library(
    fluir::compiler
    ALIAS
    fluir.libcompiler
)

set(FLUIR_COMPILER_FRONTEND_SOURCES "frontend/parser.cpp"
                                    "frontend/asg_builder.cpp"
)

set(FLUIR_COMPILER_FRONTEND_INTERFACES
    "frontend/frontend.ixx"
    "frontend/parse_tree.ixx"
    "frontend/parser.ixx"
    "frontend/asg_builder.ixx"
)

set(FLUIR_COMPILER_BACKEND_SOURCES
    "backend/bytecode_generator.cpp"
    "backend/code_writer.cpp"
    "backend/inspect_writer.cpp"
)

set(FLUIR_COMPILER_BACKEND_INTERFACES
    "backend/backend.ixx"
    "backend/bytecode_generator.ixx"
    "backend/code_writer.ixx"
    "backend/inspect_writer.ixx"
)

set(FLUIR_COMPILER_DEBUG_SOURCES "debug/asg_printer.cpp")

set(FLUIR_COMPILER_DEBUG_INTERFACES "debug/debug.ixx" "debug/asg_printer.ixx")

set(FLUIR_COMPILER_MODELS_INTERFACES
    "models/asg.ixx"
    "models/asg/node.ixx"
    "models/asg/declaration.ixx"
)

target_sources(
    fluir.libcompiler
    PRIVATE ${FLUIR_COMPILER_FRONTEND_SOURCES}
            ${FLUIR_COMPILER_BACKEND_SOURCES}
            ${FLUIR_COMPILER_DEBUG_SOURCES}
            utility/diagnostics.cpp
    PUBLIC FILE_SET
           CXX_MODULES
           FILES
           ${FLUIR_COMPILER_MODELS_INTERFACES}
           ${FLUIR_COMPILER_BACKEND_INTERFACES}
           ${FLUIR_COMPILER_DEBUG_INTERFACES}
           ${FLUIR_COMPILER_FRONTEND_INTERFACES}
)

turn_up_warnings_on(fluir.libcompiler)

target_include_directories(
    fluir.libcompiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(
    fluir.libcompiler
    PUBLIC fluir::code
           fmt::fmt
           tinyxml2::tinyxml2
)

add_executable(fluir.compiler main.cpp)

target_link_libraries(fluir.compiler PRIVATE fluir::compiler)
