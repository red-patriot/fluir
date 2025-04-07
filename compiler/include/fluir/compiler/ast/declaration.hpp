#ifndef FLUIR_COMPILER_AST_DECLARATION_HPP
#define FLUIR_COMPILER_AST_DECLARATION_HPP

#include <memory>
#include <unordered_map>

namespace fluir::ast {
  class Declaration {
    friend bool operator==(const Declaration&, const Declaration&) = default;

   public:
    virtual ~Declaration() = default;

    virtual bool equals(const Declaration& other) const = 0;
  };

  using UniqueDeclaration = std::unique_ptr<Declaration>;
  using Declarations = std::unordered_map<id_t, UniqueDeclaration>;

  inline bool operator==(const UniqueDeclaration& lhs, const UniqueDeclaration& rhs) {
    return lhs->equals(*rhs);
  }

}  // namespace fluir::ast

#endif
