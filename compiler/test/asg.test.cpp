#include <gtest/gtest.h>

#include "compiler/frontend/ast_builder.hpp"

TEST(TestAstBuilder, SingleEmptyFunction) {
  fluir::pt::ParseTree pt{
      .declarations = {{1,
                        fluir::pt::FunctionDecl{
                            .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100},
                            .name = "main",
                            .body = fluir::pt::EMPTY_BLOCK}}}};

  fluir::FlowGraphLocation expectedLocation{.x = 10, .y = 10, .z = 3, .width = 100, .height = 100};

  auto results = fluir::buildGraph(pt);
  auto& actual = results.value();
  auto& diagnostics = results.diagnostics();

  ASSERT_FALSE(diagnostics.containsErrors());
  EXPECT_EQ(1, actual.declarations.size());
  EXPECT_EQ(1, actual.declarations.front().id);
  EXPECT_EQ(expectedLocation, actual.declarations.front().location);
  EXPECT_EQ("main", actual.declarations.front().name);
  EXPECT_TRUE(actual.declarations.front().statements.empty());
}
