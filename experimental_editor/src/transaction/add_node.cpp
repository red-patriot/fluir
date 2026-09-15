#include "editor/transaction/add_node.hpp"

#include "editor/core/tree_path.hpp"

namespace fluir::editor {
  namespace {

    template <typename... Fs>
    struct Overloaded : Fs... {
      using Fs::operator()...;
    };

  }  // namespace

  bool AddNode::execute(pt::ParseTree& tree) {
    if (id_ == INVALID_ID) {
      return false;
    }
    pt::Block* block = blockOf(tree, parent_);
    if (block == nullptr || block->nodes.contains(id_)) {
      return false;
    }
    const auto* op = std::get_if<OperatorOption>(&params_);
    if (op != nullptr && op->op == Operator::UNKNOWN) {
      return false;
    }
    pt::Node node = std::visit(Overloaded{[&](const OperatorOption& o) -> pt::Node {
                                            if (o.arity == OperatorOption::BINARY) {
                                              return pt::Binary{.id = id_, .location = location_, .op = o.op};
                                            }
                                            return pt::Unary{.id = id_, .location = location_, .op = o.op};
                                          },
                                          [&](const ConstantOption& c) -> pt::Node {
                                            return pt::Constant{.id = id_, .location = location_, .value = c.value};
                                          }},
                               params_);
    return block->nodes.emplace(id_, std::move(node)).second;
  }

  bool AddNode::unexecute(pt::ParseTree& tree) {
    pt::Block* block = blockOf(tree, parent_);
    return block != nullptr && block->nodes.erase(id_) > 0;
  }

}  // namespace fluir::editor
