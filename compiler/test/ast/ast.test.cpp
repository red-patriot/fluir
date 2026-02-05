#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/ast_builder.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/context.hpp"
#include "test_diagnostic_sink.hpp"

TEST(TestAstBuilder, SingleEmptyFunction) {
  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink};
  fluir::pt::ParseTree pt{
    .declarations = {{1,
                      fluir::pt::FunctionDecl{.id = 1,
                                              .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100},
                                              .name = "main",
                                              .body = fluir::pt::EMPTY_BLOCK}}}};

  fluir::FlowGraphLocation expectedLocation{.x = 10, .y = 10, .z = 3, .width = 100, .height = 100};

  auto results = fluir::buildGraph(ctx, pt);
  auto& actual = results.value();

  ASSERT_FALSE(sink.containsErrors());
  EXPECT_EQ(1, actual.declarations.size());
  EXPECT_EQ(1, actual.declarations.front().id);
  EXPECT_EQ(expectedLocation, actual.declarations.front().location);
  EXPECT_EQ("main", actual.declarations.front().name);
  EXPECT_TRUE(actual.declarations.front().statements.empty());
}

TEST(TestBuildFlowGraph, SingleBinaryExprWithoutSharing) {
  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink};
  fluir::pt::Block block = {
    .nodes = {{1,
               fluir::pt::Binary{
                 .id = 1, .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7}, .op = fluir::Operator::STAR}},
              {2,
               fluir::pt::Constant{
                 .id = 2, .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5}, .value = fluir::pt::F64{5.6}}},
              {3,
               fluir::pt::Constant{.id = 3,
                                   .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                   .value = fluir::pt::F64{-4.7}}}},
    .conduits = {
      {4, fluir::pt::Conduit{.id = 4, .input = 2, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 0}}}},
      {5, fluir::pt::Conduit{.id = 5, .input = 3, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 1}}}},
    }};

  auto results = fluir::buildDataFlowGraph(ctx, block);
  auto& actual = results.value();

  ASSERT_FALSE(sink.containsErrors());
  ASSERT_EQ(1, actual.size());
  auto& statement = actual.at(0);

  ASSERT_TRUE(statement->is<fluir::ast::BinaryOp>());
  auto binary = statement->as<fluir::ast::BinaryOp>();

  EXPECT_EQ(fluir::Operator::STAR, binary->op());
  EXPECT_DOUBLE_EQ(5.6, binary->lhs()->as<fluir::ast::Constant>()->f64());
  EXPECT_DOUBLE_EQ(-4.7, binary->rhs()->as<fluir::ast::Constant>()->f64());
}

TEST(TestBuildFlowGraph, SingleBinaryExprWithSharing) {
  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink};
  fluir::pt::Block block = {
    .nodes = {{1,
               fluir::pt::Binary{
                 .id = 1, .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7}, .op = fluir::Operator::STAR}},
              {2,
               fluir::pt::Constant{
                 .id = 2, .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5}, .value = fluir::pt::F64{5.6}}},
              {3,
               fluir::pt::Unary{.id = 3,
                                .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                .op = fluir::Operator::PLUS}}},
    .conduits = {
      {4, fluir::pt::Conduit{.id = 4, .input = 2, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 0}}}},
      {5, fluir::pt::Conduit{.id = 5, .input = 3, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 1}}}},
      {6, fluir::pt::Conduit{.id = 6, .input = 2, .children = {fluir::pt::Conduit::Output{.target = 3, .index = 0}}}},
    }};

  auto results = fluir::buildDataFlowGraph(ctx, block);
  auto& actual = results.value();

  ASSERT_FALSE(sink.containsErrors());
  ASSERT_EQ(2, actual.size());

  {
    const auto& write = actual.at(0);
    EXPECT_TRUE(write->is<fluir::ast::LocalWrite>());
    EXPECT_EQ(2, write->as<fluir::ast::LocalWrite>()->id());
  }

  {
    auto& statement = actual.at(1);
    ASSERT_TRUE(statement->is<fluir::ast::BinaryOp>());
    auto binary = statement->as<fluir::ast::BinaryOp>();
    EXPECT_EQ(fluir::Operator::STAR, binary->op());

    {
      ASSERT_TRUE(binary->lhs()->is<fluir::ast::LocalRead>());
      const auto& lhs = binary->lhs()->as<fluir::ast::LocalRead>();
      EXPECT_EQ(2, lhs->id());
    }

    {
      ASSERT_TRUE(binary->rhs()->is<fluir::ast::UnaryOp>());
      auto unary = binary->rhs()->as<fluir::ast::UnaryOp>();
      EXPECT_EQ(fluir::Operator::PLUS, unary->op());
      ASSERT_TRUE(unary->operand()->is<fluir::ast::LocalRead>());
      EXPECT_EQ(2, unary->operand()->as<fluir::ast::LocalRead>()->id());
    }
  }
}

TEST(TestBuildFlowGraph, MultipleExprWithSharing) {
  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink};
  fluir::pt::Block block = {
    .nodes = {{1,
               fluir::pt::Binary{.id = 1,
                                 .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7},
                                 .lhs = 2,
                                 .rhs = 3,
                                 .op = fluir::Operator::SLASH}},
              {2,
               fluir::pt::Constant{
                 .id = 2, .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5}, .value = fluir::pt::F64{5.6}}},
              {3,
               fluir::pt::Unary{.id = 3,
                                .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                .lhs = 2,
                                .op = fluir::Operator::PLUS}},
              {4,
               fluir::pt::Unary{.id = 4,
                                .location = {.x = 15, .y = 12, .z = 0, .width = 5, .height = 5},
                                .lhs = 3,
                                .op = fluir::Operator::MINUS}}},
    .conduits = {
      {7, fluir::pt::Conduit{.id = 7, .input = 2, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 0}}}},
      {5, fluir::pt::Conduit{.id = 5, .input = 3, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 1}}}},
      {6, fluir::pt::Conduit{.id = 6, .input = 2, .children = {fluir::pt::Conduit::Output{.target = 3, .index = 0}}}},
      {8, fluir::pt::Conduit{.id = 8, .input = 3, .children = {fluir::pt::Conduit::Output{.target = 4, .index = 0}}}},
    }};

  auto results = fluir::buildDataFlowGraph(ctx, block);
  auto& actual = results.value();

  ASSERT_FALSE(sink.containsErrors());
  ASSERT_EQ(4, actual.size());
  auto statement =
    std::ranges::find_if(actual, [](const auto& statement) { return statement->template is<fluir::ast::BinaryOp>(); });
  ASSERT_NE(statement, actual.end());
  auto binary = (*statement)->as<fluir::ast::BinaryOp>();

  EXPECT_EQ(fluir::Operator::SLASH, binary->op());
  EXPECT_TRUE(binary->lhs()->is<fluir::ast::LocalRead>());
  ASSERT_TRUE(binary->rhs()->is<fluir::ast::LocalRead>());
  EXPECT_TRUE(binary->lhs()->is<fluir::ast::LocalRead>());

  auto statement2 =
    std::ranges::find_if(actual, [](const auto& statement) { return statement->template is<fluir::ast::UnaryOp>(); });
  ASSERT_NE(statement2, actual.end());
  auto unary2 = (*statement2)->as<fluir::ast::UnaryOp>();

  EXPECT_EQ(fluir::Operator::MINUS, unary2->op());
  EXPECT_TRUE(unary2->operand()->is<fluir::ast::LocalRead>());
}
