include(strict-warnings)

add_library(fluir.libvm)
add_library(
    fluir::vm
    ALIAS
    fluir.libvm
)

target_sources(
    fluir.libvm
    PRIVATE decode.cpp
            decoder/inspect.cpp
            vm.cpp
)

turn_up_warnings_on(fluir.libvm)

target_include_directories(
    fluir.libvm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(fluir.libvm PUBLIC fluir::code)
