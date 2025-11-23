#include "compiler/frontend/type_checker.hpp"

#include <gtest/gtest.h>

#include "compiler/types/builtin_symbols.hpp"

namespace fa = fluir::asg;
namespace ft = fluir::types;

TEST(TestDeclaractionTypeChecker, HandlesSingleConstant) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  decl.statements.emplace_back(std::make_unique<fa::Constant>(1., 1, fluir::FlowGraphLocation{}));

  fluir::Context ctx{.symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_F64;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());
}

TEST(TestDeclaractionTypeChecker, HandlesBinaryExpressionWithoutSharingNoCasts) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto lhs = std::make_shared<fa::Constant>(1.0, 1, fluir::FlowGraphLocation{});
  auto rhs = std::make_shared<fa::Constant>(2.0, 2, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::BinaryOp>(fluir::Operator::PLUS, lhs, rhs, 3, fluir::FlowGraphLocation{}));

  fluir::Context ctx{.symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_F64;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  EXPECT_FALSE(ctx.diagnostics.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());

  const auto& concrete = actual->as<fa::BinaryOp>();
  ASSERT_TRUE(concrete) << "If this fails something has gone horribly wrong";
  EXPECT_EQ(expected, concrete->lhs()->type());
  EXPECT_EQ(expected, concrete->rhs()->type());
}

TEST(TestDeclaractionTypeChecker, HandlesUnaryExpressionWithoutSharingNoCasts) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto operand =
    std::make_shared<fa::Constant>(static_cast<fluir::literals_types::U8>(7), 1, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::UnaryOp>(fluir::Operator::PLUS, operand, 3, fluir::FlowGraphLocation{}));

  fluir::Context ctx{.symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_U8;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  EXPECT_FALSE(ctx.diagnostics.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());

  const auto& concrete = actual->as<fa::UnaryOp>();
  ASSERT_TRUE(concrete) << "If this fails something has gone horribly wrong";
  EXPECT_EQ(expected, concrete->operand()->type());
}

TEST(TestDeclaractionTypeChecker, HandlesFunctionDecl) {
  fa::ASG asg{.declarations = {}};
  asg.declarations.push_back([&]() { return fa::Declaration{.id = 1, .name = "test", .statements = {}}; }());
  auto& decl = asg.declarations.front();
  auto operand =
    std::make_shared<fa::Constant>(static_cast<fluir::literals_types::U8>(7), 1, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::UnaryOp>(fluir::Operator::PLUS, operand, 3, fluir::FlowGraphLocation{}));

  fluir::Context ctx{.symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_U8;

  const auto result = fluir::typeCheck(ctx, std::move(asg));
  EXPECT_FALSE(ctx.diagnostics.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->declarations.front().statements.front();

  EXPECT_EQ(expected, actual->type());

  const auto& concrete = actual->as<fa::UnaryOp>();
  ASSERT_TRUE(concrete) << "If this fails something has gone horribly wrong";
  EXPECT_EQ(expected, concrete->operand()->type());
}
