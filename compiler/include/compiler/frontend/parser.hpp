#ifndef FLUIR_COMPILER_FRONTEND_PARSER_HPP
#define FLUIR_COMPILER_FRONTEND_PARSER_HPP

#include <string>

#include <tinyxml2.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/results.hpp"

namespace fluir {
  Results<pt::ParseTree> parseString(const std::string_view source);

  class Parser {
   public:
    Results<pt::ParseTree> parseString(const std::string_view source);

   private:
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
                                  std::string_view attribute) const;
    ID parseId(Element* element, std::string_view type) const;
    FlowGraphLocation parseLocation(Element* element, std::string_view type) const;
  };
}  // namespace fluir

#endif
