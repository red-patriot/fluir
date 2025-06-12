module;
#include <filesystem>
#include <string>
#include <utility>

#include <tinyxml2.h>

#include "compiler/utility/results.hpp"

export module fluir.frontend.parser;

import fluir.models.id;
import fluir.models.location;
import fluir.models.operators;
import fluir.frontend.parse_tree;

namespace fluir {
  export Results<pt::ParseTree> parseString(const std::string_view source);
  export Results<pt::ParseTree> parseFile(const std::filesystem::path& source);

  export class Parser {
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
    std::pair<ID, pt::Node> node(Element* element);
    std::pair<ID, pt::Node> constant(Element* element);
    std::pair<ID, pt::Node> binary(Element* element);
    std::pair<ID, pt::Node> unary(Element* element);

    std::pair<ID, pt::Conduit> conduit(Element* element);
    pt::Conduit::Output conduitOutput(Element* element);

    pt::Literal literal(Element* element);
    pt::Float fl_float(Element* element);

    std::string_view getAttribute(Element* element, std::string_view type, std::string_view attribute);
    std::string_view getOptionalAttribute(Element* element,
                                          std::string_view attribute,
                                          std::string_view defaultValue = "");
    ID parseId(Element* element, std::string_view type);
    ID parseIdReference(Element* element, std::string_view attribute, std::string_view type);
    ID parseOptionalIdReference(Element* element, std::string_view attribute, std::string_view type);
    FlowGraphLocation parseLocation(Element* element, std::string_view type);
    Operator parseOperator(Element* element, std::string_view attribute, std::string_view type);

    template <typename... FmtArgs>
    void panicIf(bool condition, Element* element, std::string_view format, FmtArgs... args);
    template <typename... FmtArgs>
    [[noreturn]] void panicAt(Element* element, std::string_view format, FmtArgs... args);
    /** Indicates the parser is in a panic */
    class PanicMode { };

    /** Line and file information for a Diagnostic.
     * Used to indicate a file's syntax is not correct in some way.
     */
    class SourceLocation : public Diagnostic::Location {
     public:
      SourceLocation(int line, std::string file) : lineNo(line), filename(std::move(file)) { }
      std::string str() const override;

     private:
      int lineNo;           /**< The line number of the element at which the diagnostic originates */
      std::string filename; /**< The file from which the diagnostic originates */
    };
  };
}  // namespace fluir
