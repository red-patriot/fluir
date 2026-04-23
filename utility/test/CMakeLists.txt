find_package(GTest REQUIRED)

add_executable(fluir.utility.test)

target_sources(fluir.utility.test PRIVATE trie.test.cpp)

target_link_libraries(
    fluir.utility.test
    PRIVATE fluir::utility
            GTest::gtest_main
            GTest::gtest
)
