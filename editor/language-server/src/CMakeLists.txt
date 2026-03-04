find_path(ASIO_INCLUDE_DIR NAMES asio.hpp REQUIRED)

add_library(
    asio::asio
    INTERFACE
    IMPORTED
)
target_include_directories(asio::asio INTERFACE ${ASIO_INCLUDE_DIR})
target_compile_definitions(asio::asio INTERFACE ASIO_STANDALONE)

include(strict-warnings)

add_executable(fluir.lsp main.cpp)

turn_up_warnings_on(fluir.lsp)

target_link_libraries(fluir.lsp PRIVATE asio::asio)
