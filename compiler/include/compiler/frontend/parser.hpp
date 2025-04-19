#ifndef FLUIR_COMPILER_FRONTEND_PARSER_HPP
#define FLUIR_COMPILER_FRONTEND_PARSER_HPP

#include <filesystem>
#include <string>

#include <tinyxml2.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/results.hpp"

namespace fluir {
  Results<pt::ParseTree> parseString(const std::string_view source);

  class Parser {
   public:
    Results<pt::ParseTree> parseString(const std::string_view source);
    Results<pt::ParseTree> parseFile(const std::filesystem::path& file);

   private:
    std::string filename_;
    tinyxml2::XMLDocument doc_;
    pt::ParseTree tree_;
    Diagnostics diagnostics_;

    using Element = tinyxml2::XMLElement;

    void flowGraph();
    void declaration(Element* element);
    void functionDecl(Element* element);

    pt::Block block(Element* element);

    std::string_view getAttribute(Element* element,
                                  std::string_view type,
                                  std::string_view attribute);
    ID parseId(Element* element, std::string_view type);
    FlowGraphLocation parseLocation(Element* element, std::string_view type);

    template <typename... FmtArgs>
    void panicIf(bool condition, Element* element, std::string_view format, FmtArgs&&... args);
    template <typename... FmtArgs>
    void panicAt(Element* element, std::string_view format, FmtArgs&&... args);
    /** Indicates the parser is in a panic */
    class PanicMode { };

    /** Line and file information for a Diagnostic.
     * Used to indicate a file's syntax is not correct in some way.
     */
    class SourceLocation : public Diagnostic::Location {
     public:
      SourceLocation(int line, std::string file) :
          lineNo(line),
          filename(std::move(file)) { }
      std::string str() const override;

     private:
      int lineNo;           /**< The line number of the element at which the diagnostic originates */
      std::string filename; /**< The file from which the diagnostic originates */
    };
  };
}  // namespace fluir

#endif
