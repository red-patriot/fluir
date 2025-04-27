include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL
          ""
          FORCE
)
FetchContent_MakeAvailable(googletest)

add_executable(fluir.vm.test)

target_sources(fluir.vm.test PRIVATE decode.test.cpp)

target_link_libraries(
    fluir.vm.test
    PRIVATE fluir::vm
            GTest::gtest
            GTest::gtest_main
)
