add_library(fluir.utility STATIC)

target_sources(fluir.utility PRIVATE "src/testing/test_files_dir.cpp")

add_library(
    fluir::utility
    ALIAS
    fluir.utility
)

target_include_directories(
    fluir.utility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(
    fluir.utility
    PRIVATE "TEST_PROGRAMS_DIRECTORY=\"${TEST_PROGRAMS_DIRECTORY}\""
)

add_subdirectory(test)
