#include "compiler/frontend/type_checker.hpp"

#include <gtest/gtest.h>

#include "compiler/types/builtin_symbols.hpp"

namespace fa = fluir::asg;
namespace ft = fluir::types;

TEST(TestDeclaractionTypeChecker, HandlesSingleConstant) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  decl.statements.emplace_back(std::make_unique<fa::Constant>(1., fluir::FullID{1, 1}, fluir::FlowGraphLocation{}));

  fluir::Context ctx{.symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_F64;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());
}

TEST(TestDeclaractionTypeChecker, HandlesBinaryExpressionWithoutSharingNoCasts) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto lhs = std::make_shared<fa::Constant>(1.0, fluir::FullID{1, 1}, fluir::FlowGraphLocation{});
  auto rhs = std::make_shared<fa::Constant>(2.0, fluir::FullID{1, 2}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::BinaryOp>(fluir::Operator::PLUS, lhs, rhs, fluir::FullID{1, 3}, fluir::FlowGraphLocation{}));

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

TEST(TestDeclaractionTypeChecker, HandlesBinaryExpressionWithoutSharingLHSCast) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto lhs =
    std::make_shared<fa::Constant>((fluir::literals_types::I16)13, fluir::FullID{1, 1}, fluir::FlowGraphLocation{});
  auto rhs = std::make_shared<fa::Constant>(2.0, fluir::FullID{1, 2}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::BinaryOp>(fluir::Operator::MINUS, lhs, rhs, fluir::FullID{1, 3}, fluir::FlowGraphLocation{}));

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
  EXPECT_TRUE(concrete->lhs()->is<fa::Cast>());
}

TEST(TestDeclaractionTypeChecker, HandlesBinaryExpressionWithoutSharingRHSCast) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto lhs = std::make_shared<fa::Constant>(1.0, fluir::FullID{1, 1}, fluir::FlowGraphLocation{});
  auto rhs =
    std::make_shared<fa::Constant>((fluir::literals_types::I32)12, fluir::FullID{1, 2}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::BinaryOp>(fluir::Operator::PLUS, lhs, rhs, fluir::FullID{1, 3}, fluir::FlowGraphLocation{}));

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
  EXPECT_TRUE(concrete->rhs()->is<fa::Cast>());
}

TEST(TestDeclaractionTypeChecker, HandlesBinaryExpressionWithSharingAndCasts) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto n1 = std::make_shared<fa::Constant>(1.0, fluir::FullID{1, 1}, fluir::FlowGraphLocation{});
  auto n2 = std::make_shared<fa::Constant>(
    static_cast<fluir::literals_types::U32>(2), fluir::FullID{1, 2}, fluir::FlowGraphLocation{});
  auto n3 =
    std::make_shared<fa::BinaryOp>(fluir::Operator::MINUS, n1, n2, fluir::FullID{1, 3}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::BinaryOp>(fluir::Operator::PLUS, n3, n2, fluir::FullID{1, 4}, fluir::FlowGraphLocation{}));

  fluir::Context ctx{.symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_F64;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  EXPECT_FALSE(ctx.diagnostics.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());

  const auto& concrete = actual->as<fa::BinaryOp>();
  ASSERT_TRUE(concrete) << "If this fails something has gone horribly wrong";
  EXPECT_TRUE(concrete->lhs()->is<fa::BinaryOp>());
  EXPECT_EQ(expected, concrete->lhs()->type());
  EXPECT_TRUE(concrete->rhs()->is<fa::Cast>());
  EXPECT_EQ(expected, concrete->rhs()->type());
  const auto& lhs = concrete->lhs()->as<fa::BinaryOp>();
  EXPECT_EQ(expected, lhs->lhs()->type());
  EXPECT_EQ(expected, lhs->rhs()->type());
  EXPECT_EQ(expected, lhs->rhs()->is<fa::Cast>());
}

TEST(TestDeclaractionTypeChecker, HandlesUnaryExpressionWithoutSharingNoCasts) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto operand = std::make_shared<fa::Constant>(
    static_cast<fluir::literals_types::U8>(7), fluir::FullID{1, 1}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::UnaryOp>(fluir::Operator::PLUS, operand, fluir::FullID{1, 3}, fluir::FlowGraphLocation{}));

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
  auto operand = std::make_shared<fa::Constant>(
    static_cast<fluir::literals_types::U8>(7), fluir::FullID{1, 1}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::UnaryOp>(fluir::Operator::PLUS, operand, fluir::FullID{1, 3}, fluir::FlowGraphLocation{}));

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
