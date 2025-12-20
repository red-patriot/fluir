add_library(fluir.utility INTERFACE)

add_library(
    fluir::utility
    ALIAS
    fluir.utility
)

target_include_directories(
    fluir.utility INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(test)
