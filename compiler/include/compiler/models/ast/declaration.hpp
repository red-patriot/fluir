#ifndef FLUIR_COMPILER_MODELS_AST_DECLARATION_HPP
#define FLUIR_COMPILER_MODELS_AST_DECLARATION_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "compiler/models/ast/node.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"

namespace fluir::ast {
  struct FunctionDecl {
    ID id;
    FlowGraphLocation location;
    std::string name;
    DataFlowGraph statements;

    std::unordered_map<ID, std::string> parameters{};
    std::optional<ID> returnValue{};  // TODO: Support multiple returns
  };

  using Declaration = FunctionDecl;  // TODO: Support other declarations
}  // namespace fluir::ast

#endif
