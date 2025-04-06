#ifndef FLUIR_COMPILER_AST_FUNCTION_DECLARATION_HPP
#define FLUIR_COMPILER_AST_FUNCTION_DECLARATION_HPP

#include "fluir/compiler/ast/declaration.hpp"
#include "fluir/compiler/ast/utility.hpp"

namespace fluir::ast {
  class FunctionDecl : public Declaration {
    friend bool operator==(const FunctionDecl&, const FunctionDecl&) = default;
    friend UniqueDeclaration makeFunctionDecl(std::string_view name, fluir::id_t id,
                                              LocationInfo params);

   public:
    FunctionDecl(std::string_view name, fluir::id_t id, LocationInfo location);

    bool equals(const Declaration& other) const override;

   private:
    std::string name_;
    fluir::id_t id_;
    LocationInfo location_;
  };

  inline UniqueDeclaration makeFunctionDecl(std::string_view name, fluir::id_t id,
                                            LocationInfo location = {}) {
    return std::make_unique<FunctionDecl>(name, id, location);
  }
}  // namespace fluir::ast

#endif
