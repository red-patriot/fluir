include(strict-warnings)

add_library(fluir.libvm)
add_library(
    fluir::vm
    ALIAS
    fluir.libvm
)

target_sources(
    fluir.libvm
    PRIVATE "decoder/decode.cpp"
            "decoder/inspect.cpp"
            "vm.cpp"
    PUBLIC FILE_SET
           CXX_MODULES
           FILES
           "vm.ixx"
           "decoder/decode.ixx"
           "decoder/inspect.ixx"
)

turn_up_warnings_on(fluir.libvm)

target_include_directories(
    fluir.libvm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(fluir.libvm PUBLIC fluir::code)

add_executable(fluir.vm main.cpp)

turn_up_warnings_on(fluir.vm)

target_link_libraries(fluir.vm PRIVATE fluir::vm)
