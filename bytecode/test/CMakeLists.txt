find_package(GTest REQUIRED)

add_executable(fluir.bytecode.test)

target_sources(fluir.bytecode.test PRIVATE value.test.cpp)

target_include_directories(
    fluir.bytecode.test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    fluir.bytecode.test
    PRIVATE fluir::code
            GTest::gtest
            GTest::gtest_main
)
