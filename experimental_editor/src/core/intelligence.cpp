#include "editor/core/intelligence.hpp"

#include <variant>

#include "editor/core/tree_path.hpp"

namespace fluir::editor {
  namespace {
    // TODO: Dynamically populate these
    const std::vector<fluir::Operator> kUnaryOperators{
      Operator::PLUS, Operator::MINUS, Operator::PLUS_PLUS, Operator::MINUS_MINUS, Operator::BANG};

    const std::vector<fluir::Operator> kBinaryOperators{Operator::PLUS,
                                                        Operator::MINUS,
                                                        Operator::STAR,
                                                        Operator::SLASH,
                                                        Operator::EQUAL_EQUAL,
                                                        Operator::BANG_EQUAL,
                                                        Operator::GREATER_EQUAL,
                                                        Operator::LESS_EQUAL,
                                                        Operator::GREATER,
                                                        Operator::LESS,
                                                        Operator::AND_AND,
                                                        Operator::BAR_BAR};

  }  // namespace

  std::vector<fluir::Operator> Intelligence::operators(const pt::ParseTree& tree, const FullID& path) const {
    const pt::Node* node = nodeAt(tree, path);
    if (node == nullptr) {
      return {};
    }
    if (std::holds_alternative<pt::Unary>(*node)) {
      return kUnaryOperators;
    }
    if (std::holds_alternative<pt::Binary>(*node)) {
      return kBinaryOperators;
    }
    return {};
  }

}  // namespace fluir::editor
