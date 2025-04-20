#include <gtest/gtest.h>

#include "compiler/frontend/ast_builder.hpp"

TEST(TestAstBuilder, SingleEmptyFunction) {
  fluir::pt::ParseTree pt{
      .declarations = {{1,
                        fluir::pt::FunctionDecl{
                            .id = 1,
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

TEST(TestBuildFlowGraph, SimpleBinaryExprWithoutSharing) {
  fluir::pt::Block block = {
      {1,
       fluir::pt::Binary{
           .id = 1,
           .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7},
           .lhs = 2,
           .rhs = 3,
           .op = fluir::Operator::STAR}},
      {2,
       fluir::pt::Constant{
           .id = 2,
           .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5},
           .value = fluir::pt::Float{5.6}}},
      {3,
       fluir::pt::Constant{
           .id = 3,
           .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
           .value = fluir::pt::Float{-4.7}}}};

  auto results = fluir::buildDataFlowGraph(block);
  auto& actual = results.value();
  auto& diagnostics = results.diagnostics();

  ASSERT_FALSE(diagnostics.containsErrors());
  ASSERT_EQ(1, actual.size());
  auto& statement = actual.at(0);

  ASSERT_EQ(1, std::holds_alternative<fluir::ast::BinaryOp>(statement));
  auto& binary = std::get<fluir::ast::BinaryOp>(statement);

  EXPECT_EQ(fluir::Operator::STAR, binary.op);
  EXPECT_DOUBLE_EQ(5.6, binary.lhs->as<fluir::ast::ConstantFP>().value);
  EXPECT_DOUBLE_EQ(-4.7, binary.rhs->as<fluir::ast::ConstantFP>().value);
}
