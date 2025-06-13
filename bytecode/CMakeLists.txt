add_library(fluir.code STATIC)

add_library(
    fluir::code
    ALIAS
    fluir.code
)

target_sources(
    fluir.code
    PUBLIC FILE_SET
           CXX_MODULES
           FILES
           "modules/byte_code.ixx"
           "modules/code_chunk.ixx"
           "modules/instruction.ixx"
)

target_include_directories(
    fluir.code PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
