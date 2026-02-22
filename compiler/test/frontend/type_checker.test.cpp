#include "compiler/frontend/type_checker.hpp"

#include <gtest/gtest.h>

#include "compiler/types/builtin_symbols.hpp"
#include "file_utility.hpp"
#include "test_diagnostic_sink.hpp"

namespace fa = fluir::ast;
namespace ft = fluir::types;

using fluir::FullID;
using fluir::INVALID_ID;

TEST(TestDeclarationTypeChecker, HandlesSingleConstant) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  decl.statements.emplace_back(std::make_unique<fa::Constant>(1., FullID{1, 1}, fluir::FlowGraphLocation{}));

  fluir::Context ctx{.symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_F64;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());
}

TEST(TestDeclarationTypeChecker, HandlesBinaryExpressionWithoutSharingNoCasts) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto lhs = fa::createDependency<fa::Constant>(1.0, FullID{1, 1}, fluir::FlowGraphLocation{});
  auto rhs = fa::createDependency<fa::Constant>(2.0, FullID{1, 2}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(std::make_unique<fa::BinaryOp>(
    fluir::Operator::PLUS, std::move(lhs), std::move(rhs), FullID{1, 3}, fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_F64;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  EXPECT_FALSE(sink.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());

  const auto& concrete = actual->as<fa::BinaryOp>();
  ASSERT_TRUE(concrete) << "If this fails something has gone horribly wrong";
  EXPECT_EQ(expected, concrete->lhs()->type());
  EXPECT_EQ(expected, concrete->rhs()->type());
}

TEST(TestDeclarationTypeChecker, HandlesBinaryExpressionWithoutSharingLHSCast) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto lhs =
    fa::createDependency<fa::Constant>((fluir::literals_types::I16)13, FullID{1, 1}, fluir::FlowGraphLocation{});
  auto rhs = fa::createDependency<fa::Constant>(2.0, FullID{1, 2}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(std::make_unique<fa::BinaryOp>(
    fluir::Operator::MINUS, fa::clone(lhs), fa::clone(rhs), FullID{1, 3}, fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_F64;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  EXPECT_FALSE(sink.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());

  const auto& concrete = actual->as<fa::BinaryOp>();
  ASSERT_TRUE(concrete) << "If this fails something has gone horribly wrong";
  EXPECT_EQ(expected, concrete->lhs()->type());
  EXPECT_EQ(expected, concrete->rhs()->type());
  EXPECT_TRUE(concrete->lhs()->is<fa::Cast>());
}

TEST(TestDeclarationTypeChecker, HandlesBinaryExpressionWithoutSharingRHSCast) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto lhs = fa::createDependency<fa::Constant>(1.0, FullID{1, 1}, fluir::FlowGraphLocation{});
  auto rhs =
    fa::createDependency<fa::Constant>((fluir::literals_types::I32)12, FullID{1, 2}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(std::make_unique<fa::BinaryOp>(
    fluir::Operator::PLUS, fa::clone(lhs), fa::clone(rhs), FullID{1, 3}, fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_F64;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  EXPECT_FALSE(sink.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());

  const auto& concrete = actual->as<fa::BinaryOp>();
  ASSERT_TRUE(concrete) << "If this fails something has gone horribly wrong";
  EXPECT_EQ(expected, concrete->lhs()->type());
  EXPECT_EQ(expected, concrete->rhs()->type());
  EXPECT_TRUE(concrete->rhs()->is<fa::Cast>());
}

TEST(TestDeclarationTypeChecker, HandlesBinaryExpressionWithSharingAndCasts) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto n1 = fa::createDependency<fa::Constant>(1.0, FullID{1, 1}, fluir::FlowGraphLocation{});
  auto n2 = fa::createDependency<fa::Constant>(
    static_cast<fluir::literals_types::U32>(2), FullID{1, 2}, fluir::FlowGraphLocation{});
  auto n3 = fa::createDependency<fa::BinaryOp>(
    fluir::Operator::MINUS, std::move(n1), fa::clone(n2), FullID{1, 3}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(std::make_unique<fa::BinaryOp>(
    fluir::Operator::PLUS, std::move(n3), std::move(n2), FullID{1, 4}, fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_F64;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  EXPECT_FALSE(sink.containsErrors());
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

TEST(TestDeclarationTypeChecker, HandlesUnaryExpressionWithoutSharingNoCasts) {
  fa::Declaration decl{.id = 1, .name = "test", .statements = {}};
  auto operand = fa::createDependency<fa::Constant>(
    static_cast<fluir::literals_types::U8>(7), FullID{1, 1}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::UnaryOp>(fluir::Operator::PLUS, fa::clone(operand), FullID{1, 3}, fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_U8;

  const auto result = fluir::checkDeclType(ctx, std::move(decl));
  EXPECT_FALSE(sink.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->statements.front();

  EXPECT_EQ(expected, actual->type());

  const auto& concrete = actual->as<fa::UnaryOp>();
  ASSERT_TRUE(concrete) << "If this fails something has gone horribly wrong";
  EXPECT_EQ(expected, concrete->operand()->type());
}

TEST(TestDeclarationTypeChecker, HandlesFunctionDecl) {
  fa::AST ast{.declarations = {}};
  ast.declarations.push_back([&]() { return fa::Declaration{.id = 1, .name = "test", .statements = {}}; }());
  auto& decl = ast.declarations.front();
  auto operand = fa::createDependency<fa::Constant>(
    static_cast<fluir::literals_types::U8>(7), FullID{1, 1}, fluir::FlowGraphLocation{});
  decl.statements.emplace_back(
    std::make_unique<fa::UnaryOp>(fluir::Operator::PLUS, fa::clone(operand), FullID{1, 3}, fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_U8;

  const auto result = fluir::typeCheck(ctx, std::move(ast));
  EXPECT_FALSE(sink.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->declarations.front().statements.front();

  EXPECT_EQ(expected, actual->type());

  const auto& concrete = actual->as<fa::UnaryOp>();
  ASSERT_TRUE(concrete) << "If this fails something has gone horribly wrong";
  EXPECT_EQ(expected, concrete->operand()->type());
}

TEST(TestDeclarationTypeChecker, HandlesLocalReadWrite) {
  fa::AST ast{.declarations = {}};
  ast.declarations.push_back([&]() { return fa::Declaration{.id = 1, .name = "test", .statements = {}}; }());
  auto& decl = ast.declarations.front();
  decl.statements.emplace_back(fa::createDependency<fa::LocalWrite>(
    fa::createDependency<fa::Constant>(
      static_cast<fluir::literals_types::I16>(12), FullID{1, 3}, fluir::FlowGraphLocation{}),
    fluir::FlowGraphLocation{}));
  decl.statements.emplace_back(
    fa::createDependency<fa::LocalRead>(3, FullID{1, INVALID_ID}, fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};
  const auto expected = fluir::types::ID_I16;

  const auto result = fluir::typeCheck(ctx, std::move(ast));
  EXPECT_FALSE(sink.containsErrors());
  ASSERT_TRUE(result.has_value());
  const auto& actual = result->declarations.front().statements.back();

  EXPECT_EQ(expected, actual->type());
}

TEST(TestDeclarationTypeChecker, LocalWriteFailsIfIdIsInvalid) {
  fa::AST ast{.declarations = {}};
  ast.declarations.push_back([&]() { return fa::Declaration{.id = 1, .name = "test", .statements = {}}; }());
  auto& decl = ast.declarations.front();
  decl.statements.emplace_back(fa::createDependency<fa::LocalWrite>(
    fa::createDependency<fa::Constant>(
      static_cast<fluir::literals_types::I16>(12), FullID{1, INVALID_ID}, fluir::FlowGraphLocation{}),
    fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};

  EXPECT_THROW(fluir::typeCheck(ctx, std::move(ast)), fluir::diagnostic::InternalError);
}

TEST(TestDeclarationTypeChecker, LocalReadFailsIfIdIsInvalid) {
  fa::AST ast{.declarations = {}};
  ast.declarations.push_back([&]() { return fa::Declaration{.id = 1, .name = "test", .statements = {}}; }());
  auto& decl = ast.declarations.front();
  decl.statements.emplace_back(fa::createDependency<fa::LocalWrite>(
    fa::createDependency<fa::Constant>(
      static_cast<fluir::literals_types::I16>(12), FullID{1, 3}, fluir::FlowGraphLocation{}),
    fluir::FlowGraphLocation{}));
  decl.statements.emplace_back(
    fa::createDependency<fa::LocalRead>(INVALID_ID, FullID{1, INVALID_ID}, fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};

  EXPECT_THROW(fluir::typeCheck(ctx, std::move(ast)), fluir::diagnostic::InternalError);
}

TEST(TestDeclarationTypeChecker, LocalReadFailsIfIdIsMissing) {
  fa::AST ast{.declarations = {}};
  ast.declarations.push_back([&]() { return fa::Declaration{.id = 1, .name = "test", .statements = {}}; }());
  auto& decl = ast.declarations.front();
  decl.statements.emplace_back(fa::createDependency<fa::LocalWrite>(
    fa::createDependency<fa::Constant>(
      static_cast<fluir::literals_types::I16>(12), FullID{1, 4}, fluir::FlowGraphLocation{}),
    fluir::FlowGraphLocation{}));
  decl.statements.emplace_back(
    fa::createDependency<fa::UnaryOp>(fluir::Operator::MINUS,
                                      fa::createDependency<fa::LocalRead>(3, FullID{1, 5}, fluir::FlowGraphLocation{}),
                                      FullID{1, 5},
                                      fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};

  const auto result = fluir::typeCheck(ctx, std::move(ast));
  EXPECT_TRUE(sink.containsErrors());
  EXPECT_FALSE(result.has_value());
  EXPECT_EQ(fluir::diagnostic::Code::ERROR_CANNOT_DETERMINE_TYPE_OF_LOCAL, sink.last().code);
  FullID expectedID{1, 5};
  const auto& errorID = std::get<FullID>(sink.last().location);
  EXPECT_EQ(expectedID, errorID);
}

TEST(TestDeclarationTypeChecker, HandlesFunctionDefsWithParameters) {
  fa::AST ast{.declarations = {}};
  ast.declarations.push_back([&]() {
    return fa::Declaration{.id = 1,
                           .location = fluir::FlowGraphLocation{},
                           .name = "test_func",
                           .statements = {},
                           .parameters = {{1, "a", "F64"}, {2, "b", "F64"}}};
  }());
  auto& decl = ast.declarations.front();
  decl.statements.emplace_back(
    fa::createDependency<fa::BinaryOp>(fluir::Operator::PLUS,
                                       fa::createDependency<fa::LocalRead>(1, FullID{1, 3}, fluir::FlowGraphLocation{}),
                                       fa::createDependency<fa::LocalRead>(2, FullID{1, 3}, fluir::FlowGraphLocation{}),
                                       FullID{1, 3},
                                       fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};

  const auto result = fluir::typeCheck(ctx, std::move(ast));
  ASSERT_FALSE(sink.containsErrors());

  EXPECT_EQ(fluir::types::ID_F64, decl.statements.at(0)->type());
}

TEST(TestDeclarationTypeChecker, HandlesFunctionDefsWithReturns) {
  fa::AST ast{.declarations = {}};
  ast.declarations.push_back([&]() {
    return fa::Declaration{.id = 1,
                           .location = fluir::FlowGraphLocation{},
                           .name = "test_func",
                           .statements = {},
                           .parameters = {{1, "a", "I32"}, {2, "b", "I16"}},
                           .returnValue = fa::FunctionDecl::Return{4, "I32"}};
  }());
  auto& decl = ast.declarations.front();
  decl.statements.emplace_back(fa::createDependency<fa::LocalWrite>(
    FullID{1, 4},
    fa::createDependency<fa::BinaryOp>(fluir::Operator::PLUS,
                                       fa::createDependency<fa::LocalRead>(1, FullID{1, 3}, fluir::FlowGraphLocation{}),
                                       fa::createDependency<fa::LocalRead>(2, FullID{1, 3}, fluir::FlowGraphLocation{}),
                                       FullID{1, 3},
                                       fluir::FlowGraphLocation{}),
    fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};

  const auto result = fluir::typeCheck(ctx, std::move(ast));
  ASSERT_FALSE(sink.containsErrors());

  EXPECT_EQ(fluir::types::ID_I32, decl.statements.at(0)->type());
}

TEST(TestDeclarationTypeChecker, HandlesCastingReturnValues) {
  fa::AST ast{.declarations = {}};
  ast.declarations.push_back([&]() {
    return fa::Declaration{.id = 1,
                           .location = fluir::FlowGraphLocation{},
                           .name = "test_func",
                           .statements = {},
                           .parameters = {{1, "a", "I32"}, {2, "b", "I16"}},
                           .returnValue = fa::FunctionDecl::Return{4, "I64"}};
  }());
  auto& decl = ast.declarations.front();
  decl.statements.emplace_back(fa::createDependency<fa::LocalWrite>(
    FullID{1, 4},
    fa::createDependency<fa::BinaryOp>(fluir::Operator::PLUS,
                                       fa::createDependency<fa::LocalRead>(1, FullID{1, 3}, fluir::FlowGraphLocation{}),
                                       fa::createDependency<fa::LocalRead>(2, FullID{1, 3}, fluir::FlowGraphLocation{}),
                                       FullID{1, 3},
                                       fluir::FlowGraphLocation{}),
    fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};

  const auto result = fluir::typeCheck(ctx, std::move(ast));
  ASSERT_FALSE(sink.containsErrors());

  EXPECT_EQ(fluir::types::ID_I64, decl.statements.at(0)->type());
  ASSERT_TRUE(decl.statements.front()->is<fa::LocalWrite>());
  const auto& child = decl.statements.front()->as<fa::LocalWrite>()->child();
  ASSERT_TRUE(child->is<fa::Cast>());
  const auto& cast = child->as<fa::Cast>();
  EXPECT_EQ(ft::ID_I32, cast->from());
  EXPECT_EQ(ft::ID_I64, cast->to());
}

TEST(TestDeclarationTypeChecker, FunctionsWithReturnsAndParamsHaveAType) {
  fa::AST ast{.declarations = {}};
  ast.declarations.push_back([&]() {
    return fa::Declaration{.id = 1,
                           .location = fluir::FlowGraphLocation{},
                           .name = "test_func",
                           .statements = {},
                           .parameters = {{1, "a", "I32"}, {2, "b", "I16"}},
                           .returnValue = fa::FunctionDecl::Return{4, "I32"}};
  }());
  auto& decl = ast.declarations.front();
  decl.statements.emplace_back(fa::createDependency<fa::LocalWrite>(
    FullID{1, 4},
    fa::createDependency<fa::BinaryOp>(fluir::Operator::PLUS,
                                       fa::createDependency<fa::LocalRead>(1, FullID{1, 3}, fluir::FlowGraphLocation{}),
                                       fa::createDependency<fa::LocalRead>(2, FullID{1, 3}, fluir::FlowGraphLocation{}),
                                       FullID{1, 3},
                                       fluir::FlowGraphLocation{}),
    fluir::FlowGraphLocation{}));

  fluir::test::TestDiagnosticSink sink;
  fluir::Context ctx{.diagnosticSink = sink, .symbolTable = ft::buildSymbolTable()};
  ft::FunctionType expected{{ft::ID_I32, ft::ID_I16}, ft::ID_I32};

  const auto result = fluir::typeCheck(ctx, std::move(ast));
  ASSERT_FALSE(sink.containsErrors());

  const auto functionType = ctx.symbolTable.getFunctionType(decl.type);
  ASSERT_TRUE(functionType);
  EXPECT_EQ(expected, *functionType);
}
