find_package(nlohmann_json REQUIRED)
find_path(ASIO_INCLUDE_DIR NAMES asio.hpp REQUIRED)

add_library(
    asio::asio
    INTERFACE
    IMPORTED
)
target_include_directories(asio::asio INTERFACE ${ASIO_INCLUDE_DIR})
target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE)

include(strict-warnings)

add_library(fluir.liblsp STATIC)

target_sources(fluir.liblsp PRIVATE "json_serialize.cpp")
target_include_directories(
    fluir.liblsp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
turn_up_warnings_on(fluir.liblsp)
target_link_libraries(
    fluir.liblsp
    PUBLIC asio::asio
           nlohmann_json::nlohmann_json
           fluir::compiler
)

add_executable(fluir.lsp)
target_sources(fluir.lsp PRIVATE "main.cpp")

turn_up_warnings_on(fluir.lsp)

target_link_libraries(fluir.lsp PRIVATE fluir.liblsp)
