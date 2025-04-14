#include "fluir/compiler/afg/flow_graph.hpp"

namespace fluir::afg {
  void FlowGraph::add(UniqueNode node) {
    nodes_.push_back(std::move(node));
  }

  Node& FlowGraph::at(size_t index) {
    return *nodes_.at(index);
  }

  const Node& FlowGraph::at(size_t index) const {
    return *nodes_.at(index);
  }
}  // namespace fluir::afg
