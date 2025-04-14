#ifndef FLUIR_COMPILER_AFG_FLOW_GRAPH_HPP
#define FLUIR_COMPILER_AFG_FLOW_GRAPH_HPP

#include <vector>

#include "fluir/compiler/afg/node.hpp"

namespace fluir::afg {
  class FlowGraph {
    friend bool operator==(const FlowGraph&, const FlowGraph&) = default;

   public:
    void add(UniqueNode node);

    Node& at(size_t index);
    const Node& at(size_t index) const;

    size_t topLevelCount() const { return nodes_.size(); }

   private:
    std::vector<UniqueNode> nodes_{};
  };
}  // namespace fluir::afg

#endif
