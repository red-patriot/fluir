#ifndef FLUIR_COMPILER_AST_DECLARATION_HPP
#define FLUIR_COMPILER_AST_DECLARATION_HPP

#include <memory>
#include <vector>

namespace fluir::ast {
  class Declaration {
    friend bool operator==(const Declaration&, const Declaration&) = default;

   public:
    virtual ~Declaration() = default;

    virtual bool equals(const Declaration& other) const = 0;
  };

  using UniqueDeclaration = std::unique_ptr<Declaration>;
  using Declarations = std::vector<UniqueDeclaration>;

  inline bool operator==(const UniqueDeclaration& lhs, const UniqueDeclaration& rhs) {
    return lhs->equals(*rhs);
  }

}  // namespace fluir::ast

#endif
