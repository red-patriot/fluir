#pragma once

#include <string_view>
#include <vector>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/operator.hpp"

namespace fluir::editor {
  struct FunctionDefOption { };
  struct CommentOption { };
  struct OperatorOption {
    Operator op;
    enum Arity { UNARY, BINARY } arity;  // TODO: Generalize this?
  };
  struct ConstantOption {
    literals_types::Literal value;
  };

  using CompletionOption = std::variant<FunctionDefOption, CommentOption, OperatorOption, ConstantOption>;

  struct Completion {
    std::string_view label;
    CompletionOption option;
  };

  /** Answers what may go where in a tree. Type- and module-aware answers slot in behind the same calls. */
  class Intelligence {
   public:
    /** Operators the operator node at `path` may take; empty for anything else. */
    std::vector<fluir::Operator> operators(const pt::ParseTree& tree, const FullID& path) const;

    /** Type names the param or return rail at `path` ([fn, railId]) may take; empty for anything else. */
    std::vector<std::string_view> types(const pt::ParseTree& tree, const FullID& path) const;

    /** Completions available inside `body`. Empty `body` indicates a top-level addition, otherwise completions will be
     * applied inside `body`. */
    std::vector<Completion> completions(const pt::ParseTree& tree, const FullID& body) const;
  };

}  // namespace fluir::editor
