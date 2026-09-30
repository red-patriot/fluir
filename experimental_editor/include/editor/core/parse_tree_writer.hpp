#pragma once

#include <ostream>

#include <tinyxml2.h>

#include "editor/core/tree.hpp"

namespace fluir::editor {

  void write(std::ostream& out, const et::ParseTree& tree);

  /** Serializes a parse tree to XML matching the editor's on-disk format. */
  class ParseTreeWriter {
   public:
    /** Writes to `out`, which the caller owns. */
    explicit ParseTreeWriter(std::ostream& out);

    void write(const et::ParseTree& tree);

    /** State of the underlying stream (false => a write or open failed). */
    [[nodiscard]] bool good() const;

   private:
    std::ostream& out_; /**< Stream out for string result */
    tinyxml2::XMLDocument doc_;

    using Element = tinyxml2::XMLElement;

    void header(Element* parent, const et::Header& value);
    void version(Element* parent, const Version& value);

    void declaration(Element* parent, const et::Declaration& value);
    void functionDecl(Element* parent, const et::FunctionDecl& value);

    void funcInputs(Element* parent, const et::FunctionDecl::InputBlock& value);
    void funcParameter(Element* parent, const et::FunctionDecl::Parameter& value);
    void funcOutputs(Element* parent, const et::FunctionDecl::OutputBlock& value);
    void funcReturn(Element* parent, const et::FunctionDecl::Return& value);

    void block(Element* parent, const et::Block& value);
    void blockContents(Element* element, const et::Block& value);
    void node(Element* parent, const et::Node& value);
    void constant(Element* parent, const et::Constant& value);
    void binary(Element* parent, const et::Binary& value);
    void unary(Element* parent, const et::Unary& value);
    void call(Element* parent, const et::Call& value);
    void conditional(Element* parent, const et::Conditional& conditional);
    void comment(Element* parent, const et::Comment& value);

    void blockPort(Element* parent, const et::BlockPort& value, std::string_view name = "port");

    void conduit(Element* parent, const et::Conduit& value);
    void conduitOutput(Element* parent, const et::Conduit::Output& value);

    void literal(Element* parent, const et::Literal& value);

    void setId(Element* element, ID id);
    void setIdReference(Element* element, ID id, std::string_view attribute);
    void setInt(Element* element, std::string_view name, int value);
    void setLocation(Element* element, const FlowGraphLocation& location);
  };

}  // namespace fluir::editor
