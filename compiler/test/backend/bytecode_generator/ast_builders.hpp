#ifndef FLUIR_COMPILER_TEST_BACKEND_BYTECODE_GENERATOR_AST_BUILDERS_HPP
#define FLUIR_COMPILER_TEST_BACKEND_BYTECODE_GENERATOR_AST_BUILDERS_HPP

#include <string>
#include <utility>
#include <vector>

#include "compiler/models/ast.hpp"
#include "compiler/models/ast/node.hpp"

namespace fluir::test {
  template <class T>
  ast::UniqueNode constant(T value, FullID id) {
    return ast::createNode<ast::Constant>(value, std::move(id), FlowGraphLocation{});
  }

  inline ast::UniqueNode binary(Operator op, ast::UniqueNode lhs, ast::UniqueNode rhs, FullID id) {
    return ast::createNode<ast::BinaryOp>(
      op, ast::Dependency{std::move(lhs)}, ast::Dependency{std::move(rhs)}, std::move(id), FlowGraphLocation{});
  }

  inline ast::UniqueNode unary(Operator op, ast::UniqueNode operand, FullID id) {
    return ast::createNode<ast::UnaryOp>(op, ast::Dependency{std::move(operand)}, std::move(id), FlowGraphLocation{});
  }

  inline ast::UniqueNode cast(types::TypeID target, ast::UniqueNode operand, FullID id) {
    return ast::createNode<ast::Cast>(target, ast::Dependency{std::move(operand)}, std::move(id), FlowGraphLocation{});
  }

  inline ast::UniqueNode localRead(ID variable, FullID id) {
    return ast::createNode<ast::LocalRead>(variable, std::move(id), FlowGraphLocation{});
  }

  inline ast::UniqueNode localWrite(ast::UniqueNode child) {
    return ast::createNode<ast::LocalWrite>(ast::Dependency{std::move(child)}, FlowGraphLocation{});
  }

  inline ast::UniqueNode localWrite(FullID id, ast::UniqueNode child) {
    return ast::createNode<ast::LocalWrite>(std::move(id), ast::Dependency{std::move(child)}, FlowGraphLocation{});
  }

  template <class... N>
  std::vector<ast::Dependency> deps(N&&... nodes) {
    std::vector<ast::Dependency> result;
    result.reserve(sizeof...(nodes));
    (result.emplace_back(std::forward<N>(nodes)), ...);
    return result;
  }

  inline ast::UniqueNode call(std::string name, std::vector<ast::Dependency> args, FullID id) {
    return ast::createNode<ast::Call>(std::move(name), std::move(args), std::move(id), FlowGraphLocation{});
  }

  inline ast::UniqueNode bundle(FullID id, std::vector<ast::Dependency> children) {
    return ast::createNode<ast::Bundle>(std::move(id), FlowGraphLocation{}, std::move(children));
  }

  inline ast::UniqueNode unbundle(FullID id, ast::UniqueNode child, size_t index) {
    return ast::createNode<ast::Unbundle>(std::move(id), FlowGraphLocation{}, ast::Dependency{std::move(child)}, index);
  }

  inline ast::ScopeInput scopeInput(ast::UniqueNode node, ID innerId) {
    return ast::ScopeInput{ast::Dependency{std::move(node)}, innerId};
  }

  template <class... S>
  std::vector<ast::ScopeInput> inputs(S&&... scopeInputs) {
    std::vector<ast::ScopeInput> result;
    result.reserve(sizeof...(scopeInputs));
    (result.push_back(std::forward<S>(scopeInputs)), ...);
    return result;
  }

  template <class... N>
  ast::SubGraph sinks(N&&... nodes) {
    ast::SubGraph graph;
    (graph.sinks.emplace_back(std::forward<N>(nodes)), ...);
    return graph;
  }

  inline ast::SubGraph output(ast::UniqueNode node) { return ast::SubGraph{.sinks = {}, .output = std::move(node)}; }

  inline ast::UniqueNode conditional(FullID id,
                                     ast::UniqueNode condition,
                                     std::vector<ast::ScopeInput> scopeInputs,
                                     std::vector<ID> outputs,
                                     ast::SubGraph thenBranch,
                                     ast::SubGraph elseBranch) {
    return ast::createNode<ast::Conditional>(std::move(id),
                                             FlowGraphLocation{},
                                             ast::Dependency{std::move(condition)},
                                             std::move(scopeInputs),
                                             std::move(outputs),
                                             std::move(thenBranch),
                                             std::move(elseBranch));
  }

  template <class... N>
  ast::FunctionDecl function(ID id, std::string name, N&&... statements) {
    ast::FunctionDecl decl{.id = id, .name = std::move(name), .statements = {}};
    (decl.statements.emplace_back(std::forward<N>(statements)), ...);
    return decl;
  }

  template <class... D>
  ast::AST program(D&&... decls) {
    ast::AST result;
    (result.declarations.emplace_back(std::forward<D>(decls)), ...);
    return result;
  }
}  // namespace fluir::test

#endif
