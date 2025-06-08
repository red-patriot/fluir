#include <gtest/gtest.h>

#include "compiler/frontend/asg_builder.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"

TEST(TestAstBuilder, SingleEmptyFunction) {
  fluir::pt::ParseTree pt{
    .declarations = {{1,
                      fluir::pt::FunctionDecl{.id = 1,
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

TEST(TestBuildFlowGraph, SingleBinaryExprWithoutSharing) {
  fluir::pt::Block block = {
    .nodes = {{1,
               fluir::pt::Binary{
                 .id = 1, .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7}, .op = fluir::Operator::STAR}},
              {2,
               fluir::pt::Constant{.id = 2,
                                   .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5},
                                   .value = fluir::pt::Float{5.6}}},
              {3,
               fluir::pt::Constant{.id = 3,
                                   .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                   .value = fluir::pt::Float{-4.7}}}},
    .conduits = {
      {4, fluir::pt::Conduit{.id = 4, .input = 2, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 0}}}},
      {5, fluir::pt::Conduit{.id = 5, .input = 3, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 1}}}},
    }};

  auto results = fluir::buildDataFlowGraph(block);
  auto& actual = results.value();
  auto& diagnostics = results.diagnostics();

  ASSERT_FALSE(diagnostics.containsErrors());
  ASSERT_EQ(1, actual.size());
  auto& statement = actual.at(0);

  ASSERT_EQ(1, std::holds_alternative<fluir::asg::BinaryOp>(statement));
  auto& binary = std::get<fluir::asg::BinaryOp>(statement);

  EXPECT_EQ(fluir::Operator::STAR, binary.op);
  EXPECT_DOUBLE_EQ(5.6, binary.lhs->as<fluir::asg::ConstantFP>().value);
  EXPECT_DOUBLE_EQ(-4.7, binary.rhs->as<fluir::asg::ConstantFP>().value);
}

TEST(TestBuildFlowGraph, SingleBinaryExprWithSharing) {
  fluir::pt::Block block = {
    .nodes = {{1,
               fluir::pt::Binary{
                 .id = 1, .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7}, .op = fluir::Operator::STAR}},
              {2,
               fluir::pt::Constant{.id = 2,
                                   .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5},
                                   .value = fluir::pt::Float{5.6}}},
              {3,
               fluir::pt::Unary{.id = 3,
                                .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                .op = fluir::Operator::PLUS}}},
    .conduits = {
      {4, fluir::pt::Conduit{.id = 4, .input = 2, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 0}}}},
      {5, fluir::pt::Conduit{.id = 5, .input = 3, .children = {fluir::pt::Conduit::Output{.target = 1, .index = 1}}}},
      {6, fluir::pt::Conduit{.id = 6, .input = 2, .children = {fluir::pt::Conduit::Output{.target = 3, .index = 0}}}},
    }};

  auto results = fluir::buildDataFlowGraph(block);
  auto& actual = results.value();
  auto& diagnostics = results.diagnostics();

  ASSERT_FALSE(diagnostics.containsErrors());
  ASSERT_EQ(1, actual.size());
  auto& statement = actual.at(0);

  ASSERT_EQ(1, std::holds_alternative<fluir::asg::BinaryOp>(statement));
  auto& binary = std::get<fluir::asg::BinaryOp>(statement);

  EXPECT_EQ(fluir::Operator::STAR, binary.op);
  EXPECT_DOUBLE_EQ(5.6, binary.lhs->as<fluir::asg::ConstantFP>().value);
  ASSERT_TRUE(binary.rhs->is<fluir::asg::UnaryOp>());
  auto& unary = binary.rhs->as<fluir::asg::UnaryOp>();
  EXPECT_EQ(fluir::Operator::PLUS, unary.op);
  EXPECT_DOUBLE_EQ(5.6, unary.operand->as<fluir::asg::ConstantFP>().value);

  EXPECT_EQ(binary.lhs, unary.operand);
}

TEST(TestBuildFlowGraph, MultipleExprWithSharing) {
  fluir::pt::Block block = {
    .nodes = {{1,
               fluir::pt::Binary{.id = 1,
                                 .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7},
                                 .lhs = 2,
                                 .rhs = 3,
                                 .op = fluir::Operator::SLASH}},
              {2,
               fluir::pt::Constant{.id = 2,
                                   .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5},
                                   .value = fluir::pt::Float{5.6}}},
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

  auto results = fluir::buildDataFlowGraph(block);
  auto& actual = results.value();
  auto& diagnostics = results.diagnostics();

  ASSERT_FALSE(diagnostics.containsErrors());
  ASSERT_EQ(2, actual.size());
  auto statement = std::ranges::find_if(
    actual, [](const auto& statement) { return std::holds_alternative<fluir::asg::BinaryOp>(statement); });
  ASSERT_NE(statement, actual.end());
  auto& binary = std::get<fluir::asg::BinaryOp>(*statement);

  EXPECT_EQ(fluir::Operator::SLASH, binary.op);
  EXPECT_DOUBLE_EQ(5.6, binary.lhs->as<fluir::asg::ConstantFP>().value);
  ASSERT_TRUE(binary.rhs->is<fluir::asg::UnaryOp>());
  auto& unary1 = binary.rhs->as<fluir::asg::UnaryOp>();
  EXPECT_EQ(fluir::Operator::PLUS, unary1.op);
  EXPECT_DOUBLE_EQ(5.6, unary1.operand->as<fluir::asg::ConstantFP>().value);

  EXPECT_EQ(binary.lhs, unary1.operand);

  auto statement2 = std::ranges::find_if(
    actual, [](const auto& statement) { return std::holds_alternative<fluir::asg::UnaryOp>(statement); });
  ASSERT_NE(statement2, actual.end());
  auto& unary2 = statement2->as<fluir::asg::UnaryOp>();

  EXPECT_EQ(fluir::Operator::MINUS, unary2.op);
  EXPECT_EQ(binary.rhs, unary2.operand);
}
