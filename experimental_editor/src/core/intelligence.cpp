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

    // TODO: Dynamically populate these
    const std::vector<std::string_view> kBuiltinTypes{
      "F64", "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "BOOL"};

    // TODO: Dynamically populate these
    const std::vector<Completion> kTopLevelCompletions{{"Function", FunctionDefOption{}}, {"Comment", CommentOption{}}};

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

  std::vector<std::string_view> Intelligence::types(const pt::ParseTree& tree, const FullID& path) const {
    if (path.empty()) {
      return {};
    }
    const pt::FunctionDecl* fn = functionAt(tree, parentOf(path));
    return fn != nullptr && railTypeAt(*fn, path.back()) != nullptr ? kBuiltinTypes : std::vector<std::string_view>{};
  }

  std::vector<Completion> Intelligence::completions(const pt::ParseTree&, const FullID& body) const {
    return body.empty() ? kTopLevelCompletions : std::vector<Completion>{};
  }

}  // namespace fluir::editor
