#include "fluir/compiler/ast/function_declaration.hpp"

namespace fluir::ast {
  FunctionDecl::FunctionDecl(std::string_view name, fluir::id_t id, LocationInfo location) :
      name_(name),
      id_(id),
      location_(location) { }

  bool FunctionDecl::equals(const Declaration& other) const {
    const FunctionDecl* concrete = dynamic_cast<const FunctionDecl*>(&other);
    return concrete && (*this) == (*concrete);
  }
}  // namespace fluir::ast
