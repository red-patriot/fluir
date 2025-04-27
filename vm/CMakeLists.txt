include(CTestUseLaunchers)
include(FetchContent)

add_subdirectory(src)

if (FLUIR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()
