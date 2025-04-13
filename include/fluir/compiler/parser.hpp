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
    /** Parse fluir source from a file into an AST */
    fluir::ast::AST parseFile(const std::filesystem::path& program);
    /** Parse fluir source from a string into an AST. */
    fluir::ast::AST parseString(const std::string_view src);
    /** Get the diagnostics of the last run */
    const DiagnosticCollection& diagnostics() const { return diagnostics_; }

   private:
    std::string filename_;               /**< The current file being parsed */
    tinyxml2::XMLDocument source_;       /**< Tokenized source code */
    DiagnosticCollection diagnostics_{}; /**< The diagnostics gathered in the current parse */
    fluir::ast::AST ast_{};              /**< The AST generated in the current parse */

    /** Resets the internal state to parse a new source */
    void reset(const std::string_view filename);

    using Element = tinyxml2::XMLElement;

    /** Parses the root element */
    void root();
    /** Parses a declaration from the element and adds it to the AST.
     * Synchronizes panics that occur internally.
     */
    void declaration(Element* element);
    /** Parses a function declaration from the element and adds it to the AST */
    void function(Element* element);

    /** Parses a block from the element.
     * Synchronizes panics that occur internally.
     */
    ast::Block block(Element* element);
    /** Parses a unary operation from the element */
    ast::Unary unary(Element* element);
    /** Parses a binary operation from the element */
    ast::Binary binary(Element* element);
    /** Parses a constant node from the element */
    ast::Constant constant(Element* element);
    /** Parses a value from the element */
    ast::Value value(Element* element);

    /** Indicates that the parser is panicking */
    class Panic { };

    /** Parses the id of the element. */
    fluir::id_t parseId(Element* element, std::string_view type);
    /** Parses the location on the flow diagram of the element */
    ast::LocationInfo parseLocation(Element* element, std::string_view type);
    /** Parses the given attribute */
    std::string_view getAttribute(Element* element, std::string_view type, std::string_view attribute);

    /** Conditionally initiates a panic.
     *
     * \param condition If this condition is false, initiates a panic, does nothing otherwise
     * \param element The element that caused the panic, used for reporting
     * \param errorMessage Format arguments for a user message if a panic occurs
     */
    template <typename... Args>
    void panicIf(bool condition, Element* element, Args&&... errorMessage);
    /** Unconditionally initiates a panic.
     *
     * \param condition If this condition is false, initiates a panic, does nothing otherwise
     * \param element The element that caused the panic, used for reporting
     * \param errorMessage Format arguments for a user message if a panic occurs
     */
    template <typename... Args>
    void panic(Element* element, Args&&... errorMessage);

    /** Line and file information for a Diagnostic.
     * Used to indicate a file's syntax is not correct in some way.
     */
    class SourceLocation : public Diagnostic::Where {
     public:
      SourceLocation(int line, std::string file) :
          lineNo(line),
          filename(std::move(file)) { }
      std::string string() const override;

     private:
      int lineNo;           /**< The line number of the element at which the diagnostic originates */
      std::string filename; /**< The file from which the diagnostic originates */
    };
  };
}  // namespace fluir::compiler

#endif
