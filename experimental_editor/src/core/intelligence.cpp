#include "editor/core/intelligence.hpp"

#include <cstddef>
#include <string>
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

    // One default-constructed (0 / false) value per Literal alternative, in order.
    template <typename... Ts>
    std::vector<literals_types::Literal> defaultsOf(const std::variant<Ts...>*) {
      return {literals_types::Literal{std::in_place_type<Ts>}...};
    }

    // Binary operators, unary operators, one default constant per builtin type, then Comment.
    const std::vector<Completion>& bodyCompletions() {
      // Completion labels are views, so the strings live here.
      static const std::vector<std::string> kOperatorLabels = [] {
        std::vector<std::string> labels;
        for (Operator op : kBinaryOperators) {
          labels.push_back(std::string{stringify(op)} + " (binary)");
        }
        for (Operator op : kUnaryOperators) {
          labels.push_back(std::string{stringify(op)} + " (unary)");
        }
        return labels;
      }();
      static const std::vector<Completion> kCompletions = [] {
        std::vector<Completion> out;
        std::size_t label = 0;
        for (Operator op : kBinaryOperators) {
          out.push_back({kOperatorLabels[label++], OperatorOption{op, OperatorOption::BINARY}});
        }
        for (Operator op : kUnaryOperators) {
          out.push_back({kOperatorLabels[label++], OperatorOption{op, OperatorOption::UNARY}});
        }
        // Builtin names follow Literal's alternative order.
        const auto defaults = defaultsOf(static_cast<const literals_types::Literal*>(nullptr));
        for (std::size_t i = 0; i < kBuiltinTypes.size(); ++i) {
          out.push_back({kBuiltinTypes[i], ConstantOption{defaults.at(i)}});
        }
        out.push_back({"Comment", CommentOption{}});
        return out;
      }();
      return kCompletions;
    }

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

  std::vector<Completion> Intelligence::completions(const pt::ParseTree& tree, const FullID& body) const {
    if (body.empty()) {
      return kTopLevelCompletions;
    }
    return blockOf(tree, body) != nullptr ? bodyCompletions() : std::vector<Completion>{};
  }

}  // namespace fluir::editor
