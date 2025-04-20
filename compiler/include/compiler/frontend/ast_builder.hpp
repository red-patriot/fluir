#ifndef FLUIR_COMPILER_FRONTEND_AST_BUILDER_HPP
#define FLUIR_COMPILER_FRONTEND_AST_BUILDER_HPP

#include "compiler/frontend/ast/ast.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/results.hpp"

namespace fluir {
  Results<ast::ASG> buildGraph(const pt::ParseTree& tree);
  //   Results<ast::AST> buildTree(const ast::ASG& graph);

  Results<ast::DataFlowGraph> buildDataFlowGraph(pt::Block block);

}  // namespace fluir

#endif
