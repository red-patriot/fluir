#ifndef FLUIR_COMPILER_PARSER_HPP
#define FLUIR_COMPILER_PARSER_HPP

#include <filesystem>
#include <string>

#include <tinyxml2.h>

#include "fluir/compiler/ast.hpp"
#include "fluir/compiler/diagnostic.hpp"

namespace fluir::compiler {
  class Parser {
   public:
    fluir::ast::AST parseFile(const std::filesystem::path& program);
    fluir::ast::AST parse(const std::string_view src);
    const DiagnosticCollection& diagnostics() const { return diagnostics_; }

   private:
    std::string filename_;
    tinyxml2::XMLDocument source_;
    DiagnosticCollection diagnostics_{};
    fluir::ast::AST ast_{};

    void reset(const std::string_view filename);

    using Element = tinyxml2::XMLElement;

    void root();
    void declaration(Element* element);
    void function(Element* element);

    ast::Block block(Element* element);
    ast::Binary binary(Element* element);
    ast::Constant constant(Element* element);
    ast::Value value(Element* element);

    class Panic { };

    fluir::id_t parseId(Element* element, std::string_view type);
    ast::LocationInfo parseLocation(Element* element, std::string_view type);
    std::string_view getAttribute(Element* element, std::string_view type, std::string_view attribute);

    template <typename... Args>
    void panicIf(bool condition, Element* element, Args&&... errorMessage);
    template <typename... Args>
    void panic(Element* element, Args&&... errorMessage);

    class SourceLocation : public Diagnostic::Where {
     public:
      SourceLocation(int line, std::string file) :
          lineNo(line),
          filename(std::move(file)) { }
      std::string string() const override;

     private:
      int lineNo;
      std::string filename;
    };
  };
}  // namespace fluir::compiler

#endif
