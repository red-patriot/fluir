find_package(GTest REQUIRED)

add_executable(fluir.lsp.test)

target_sources(
    fluir.lsp.test
    PRIVATE "json_serialize.test.cpp"
            "analysis_db.test.cpp"
            "compiler_diagnostics_shim.test.cpp"
            "symbol_tree.test.cpp"
)

target_include_directories(fluir.lsp.test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    fluir.lsp.test
    PRIVATE fluir.liblsp
            GTest::gtest
            GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(fluir.lsp.test)
