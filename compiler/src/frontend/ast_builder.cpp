#include "compiler/frontend/ast_builder.hpp"

namespace {
  class GraphBuilder {
   public:
    fluir::ast::Declaration operator()(const fluir::pt::FunctionDecl& func) {
      return fluir::ast::FunctionDecl{1,  // TODO: Don't hardcode this
                                      func.location,
                                      func.name,
                                      {}};  // TODO: Handle body
    }
  };
}  // namespace

namespace fluir {
  Results<ast::ASG> buildGraph(const pt::ParseTree& tree) {
    ast::ASG graph;
    Diagnostics diagnostics;

    for (const auto& [id, declaration] : tree.declarations) {
      graph.declarations.emplace_back(std::visit(GraphBuilder{}, declaration));
    }

    return Results<ast::ASG>{graph, diagnostics};
  }
}  // namespace fluir
