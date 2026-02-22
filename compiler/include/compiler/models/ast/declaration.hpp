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
    struct Parameter {
      ID id;
      std::string name;
      std::string typeName{};
      types::TypeID type{types::ID_INVALID};
    };

    struct Return {
      ID id;
      std::string typeName;
    };

    ID id;
    FlowGraphLocation location;
    std::string name;
    DataFlowGraph statements;
    types::TypeID type{types::ID_INVALID};

    std::vector<Parameter> parameters{};
    std::optional<Return> returnValue{};  // TODO: Support multiple returns
  };

  using Declaration = FunctionDecl;  // TODO: Support other declarations
}  // namespace fluir::ast

#endif
