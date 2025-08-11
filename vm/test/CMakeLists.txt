find_package(GTest REQUIRED)

add_executable(fluir.vm.test)

target_sources(
    fluir.vm.test
    PRIVATE decoder/decode.test.cpp
            decoder/inspect.test.cpp
            vm.test.cpp
)

target_include_directories(fluir.vm.test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    fluir.vm.test
    PRIVATE fluir::vm
            GTest::gtest
            GTest::gtest_main
)
