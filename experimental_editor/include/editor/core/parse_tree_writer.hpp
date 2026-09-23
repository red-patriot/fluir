#pragma once

#include <ostream>

#include <tinyxml2.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"

namespace fluir::editor {

  void write(std::ostream& out, const fluir::pt::ParseTree& tree);

  /** Serializes a parse tree to XML matching the editor's on-disk format. */
  class ParseTreeWriter {
   public:
    /** Writes to `out`, which the caller owns. */
    explicit ParseTreeWriter(std::ostream& out);

    void write(const fluir::pt::ParseTree& tree);

    /** State of the underlying stream (false => a write or open failed). */
    [[nodiscard]] bool good() const;

   private:
    std::ostream& out_; /**< Stream out for string result */
    tinyxml2::XMLDocument doc_;

    using Element = tinyxml2::XMLElement;

    void header(Element* parent, const pt::Header& value);
    void version(Element* parent, const Version& value);

    void declaration(Element* parent, const pt::Declaration& value);
    void functionDecl(Element* parent, const pt::FunctionDecl& value);

    void funcInputs(Element* parent, const pt::FunctionDecl::InputBlock& value);
    void funcParameter(Element* parent, const pt::FunctionDecl::Parameter& value);
    void funcOutputs(Element* parent, const pt::FunctionDecl::OutputBlock& value);
    void funcReturn(Element* parent, const pt::FunctionDecl::Return& value);

    void block(Element* parent, const pt::Block& value);
    void blockContents(Element* element, const pt::Block& value);
    void node(Element* parent, const pt::Node& value);
    void constant(Element* parent, const pt::Constant& value);
    void binary(Element* parent, const pt::Binary& value);
    void unary(Element* parent, const pt::Unary& value);
    void call(Element* parent, const pt::Call& value);
    void conditional(Element* parent, const pt::Conditional& conditional);
    void comment(Element* parent, const pt::Comment& value);

    void blockPort(Element* parent, const pt::BlockPort& value, std::string_view name = "port");

    void conduit(Element* parent, const pt::Conduit& value);
    void conduitOutput(Element* parent, const pt::Conduit::Output& value);

    void literal(Element* parent, const pt::Literal& value);

    void setId(Element* element, ID id);
    void setIdReference(Element* element, ID id, std::string_view attribute);
    void setInt(Element* element, std::string_view name, int value);
    void setLocation(Element* element, const FlowGraphLocation& location);
  };

}  // namespace fluir::editor
