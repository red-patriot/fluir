module;
#include <unordered_map>
#include <vector>

export module fluir.frontend.asg_builder;

import fluir.models.asg;
import fluir.frontend.parse_tree;
import fluir.utility.results;

namespace fluir {
  export Results<asg::ASG> buildGraph(const pt::ParseTree& tree);

  export Results<asg::DataFlowGraph> buildDataFlowGraph(pt::Block block);

  export class ASGBuilder {
   public:
    static Results<asg::ASG> buildFrom(const pt::ParseTree& tree);

    fluir::asg::Declaration operator()(const fluir::pt::FunctionDecl& func);

   private:
    const pt::ParseTree& tree_;
    asg::ASG graph_;
    Diagnostics diagnostics_;

    Results<asg::ASG> run();

    explicit ASGBuilder(const pt::ParseTree& tree);
  };

  export class FlowGraphBuilder {
   public:
    static Results<asg::DataFlowGraph> buildFrom(pt::Block block);

    asg::Node operator()(const pt::Binary& pt);
    asg::Node operator()(const pt::Unary& pt);
    asg::Node operator()(const pt::Constant& pt);

   private:
    asg::DataFlowGraph graph_;
    pt::Block block_;
    Diagnostics diagnostics_;
    std::unordered_map<ID, asg::SharedDependency> alreadyFound_;
    std::vector<ID> inProgressNodes_;

    explicit FlowGraphBuilder(pt::Block block);

    Results<asg::DataFlowGraph> run();

    asg::SharedDependency getDependency(ID dependentId, int index);
  };
}  // namespace fluir
