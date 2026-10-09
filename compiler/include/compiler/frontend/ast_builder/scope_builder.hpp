#ifndef FLUIR_COMPILER_FRONTEND_AST_BUILDER_SCOPE_BUILDER_HPP
#define FLUIR_COMPILER_FRONTEND_AST_BUILDER_SCOPE_BUILDER_HPP

#include <unordered_set>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/ast.hpp"
#include "compiler/utility/context.hpp"
#include "compiler/utility/topological_sort.hpp"

namespace fluir::fe {
  /** Builds the AST for a single scope */
  class ScopeBuilder {
   public:
    static Results<ast::DataFlowGraph> buildAst(Context& ctx,
                                                const pt::Block& block,
                                                const std::unordered_set<ID>& inputs,
                                                const std::unordered_set<ID>& outputs,
                                                FullID id);

    ast::UniqueNode operator()(const pt::Binary& pt);
    ast::UniqueNode operator()(const pt::Unary& pt);
    ast::UniqueNode operator()(const pt::Constant& pt);
    ast::UniqueNode operator()(const pt::Call& pt);
    ast::UniqueNode operator()(const pt::Comment&) { return nullptr; }
    ast::UniqueNode operator()(const pt::Conditional& pt);

   private:
    Context& ctx_;
    ast::DataFlowGraph graph_;
    pt::Block pt_;
    FullID currentID_{};
    std::unordered_set<ID> alreadyFound_;
    std::unordered_set<ID> promotedIds_;
    std::unordered_set<ID> inputs_;
    std::unordered_set<ID> outputs_;
    std::vector<ID> inProgressNodes_;
    dag::NodeSet<ID> dependencies_;

    explicit ScopeBuilder(Context& ctx,
                          pt::Block block,
                          const std::unordered_set<ID>& inputs,
                          const std::unordered_set<ID>& outputs,
                          FullID id);

    Results<ast::DataFlowGraph> run();

    /** Process the given parse tree node into an AST subtree */
    ast::UniqueNode process(ID id, pt::Node pt);
    /** Returns the dependency node with the given ID+index, processing it if necessary */
    ast::Dependency getDependency(ID dependentId, std::optional<unsigned> index);

    /** Returns the IDs of all nodes that are promoted to local variables because they have multiple dependents*/
    std::unordered_set<ID> getIdsPromotedToLocal() const;
    /** Returns the IDs of all nodes that have no dependents. */
    std::unordered_set<ID> getSinkIds() const;
  };
}  // namespace fluir::fe

#endif
