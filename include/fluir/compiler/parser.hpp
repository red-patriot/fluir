#ifndef FLUIR_COMPILER_PARSER_HPP
#define FLUIR_COMPILER_PARSER_HPP

#include <string>

#include <tinyxml2.h>

#include "fluir/compiler/ast.hpp"
#include "fluir/compiler/diagnostic.hpp"

namespace fluir::compiler {
  class Parser {
   public:
    fluir::ast::AST parse(const std::string_view src);
    const DiagnosticCollection& diagnostics() const { return diagnostics_; }

   private:
    tinyxml2::XMLDocument source_;
    DiagnosticCollection diagnostics_{};
    fluir::ast::AST ast_{};

    using Element = tinyxml2::XMLElement;

    void root();
    void declaration(Element* element);
    void function(Element* element);

    ast::Block block(Element* element);
    ast::Constant constant(Element* element);
    ast::Value value(Element* element);

    ast::LocationInfo parseLocation(Element* element);
  };
}  // namespace fluir::compiler

#endif
