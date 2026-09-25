#include "editor/transaction/add_node.hpp"

#include <algorithm>
#include <memory>
#include <ranges>
#include <utility>

#include "editor/core/tree_path.hpp"
#include "fluir/util/overloaded.hpp"

namespace fluir::editor {
  bool AddNode::execute(et::ParseTree& tree) {
    if (id_ == INVALID_ID) {
      return false;
    }
    et::Block* block = blockOf(tree, parent_);
    if (block == nullptr || block->nodes.contains(id_)) {
      return false;
    }
    const auto* op = std::get_if<OperatorOption>(&params_);
    if (op != nullptr && op->op == Operator::UNKNOWN) {
      return false;
    }
    et::Node node =
      std::visit(util::Overloaded{[&](const OperatorOption& o) -> et::Node {
                                    if (o.arity == OperatorOption::BINARY) {
                                      return et::Binary{.id = id_, .location = location_, .op = o.op};
                                    }
                                    return et::Unary{.id = id_, .location = location_, .op = o.op};
                                  },
                                  [&](const ConstantOption& c) -> et::Node {
                                    return et::Constant{.id = id_, .location = location_, .value = c.value};
                                  },
                                  [&](const CallFunctionOption& call) -> et::Node {
                                    namespace rv = std::ranges::views;

                                    et::Call::Arguments args;
                                    args.reserve(call.parameters.size());
                                    std::ranges::copy(
                                      call.parameters | rv::enumerate | rv::transform([](const auto& c) {
                                        const auto& [i, param] = c;
                                        return et::Call::Argument{.name = param.name, .index = static_cast<int>(i)};
                                      }),
                                      std::back_inserter(args));

                                    std::optional<et::Call::Return> ret;
                                    if (call.returnType) {
                                      ret = et::Call::Return{};
                                    }

                                    return et::Call{.id = id_,
                                                    .location = location_,
                                                    .target = std::string{call.target},
                                                    ._return = ret,
                                                    .arguments = std::move(args)};
                                  },
                                  [&](const ConditionalOption&) -> et::Node {
                                    return et::Conditional{
                                      .id = id_,
                                      .location = location_,
                                      .condition = {},
                                      .inputs = {},
                                      .outputs = {},
                                      .thenScope = xyz::indirect<et::Block>{},
                                      .elseScope = xyz::indirect<et::Block>{},
                                    };
                                  }},
                 params_);
    return block->nodes.emplace(id_, std::move(node)).second;
  }

  bool AddNode::unexecute(et::ParseTree& tree) {
    et::Block* block = blockOf(tree, parent_);
    return block != nullptr && block->nodes.erase(id_) > 0;
  }

  std::unique_ptr<Transaction> addNode(fluir::FullID parent,
                                       fluir::ID newId,
                                       fluir::FlowGraphLocation location,
                                       AddNode::Params params) {
    return std::make_unique<AddNode>(std::move(parent), newId, location, std::move(params));
  }

}  // namespace fluir::editor
