#include "editor/app/style.hpp"

#include <gtest/gtest.h>

TEST(Style, EditorStyleIsLoadedFromTheResources) { EXPECT_FALSE(fluir::editor::editorStyle().isEmpty()); }
