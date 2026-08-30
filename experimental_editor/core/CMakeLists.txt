include(strict-warnings)

add_library(fluir.editor.core src/placeholder.cpp)
add_library(
    fluir::editor-core
    ALIAS
    fluir.editor.core
)

target_include_directories(fluir.editor.core PUBLIC include)

target_link_libraries(fluir.editor.core PUBLIC fluir::compiler)

turn_up_warnings_on(fluir.editor.core)

if (FLUIR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()
