#include "editor/core/parse_tree_writer.hpp"

#include <algorithm>
#include <string>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <tinyxml2.h>

#include "compiler/models/operator.hpp"
#include "editor/core/literal_text.hpp"
#include "fluir/util/overloaded.hpp"

using namespace std::string_view_literals;

namespace fluir::editor {

  namespace {

    struct TwoSpacePrinter : tinyxml2::XMLPrinter {
      void PrintSpace(int depth) override {
        for (int i = 0; i < depth; ++i)
          Print("  ");
      }
    };

    // TODO: Figure out a way to get away from sorting
    template <typename Map>
    std::vector<fluir::ID> sortedIds(const Map& m) {
      std::vector<fluir::ID> ids;
      ids.reserve(m.size());
      for (const auto& kv : m)
        ids.push_back(kv.first);
      std::sort(ids.begin(), ids.end());
      return ids;
    }

    // Round-trip formatting, not display formatting: F64 keeps `{:f}` so the
    // written text reparses as the same value (`renderLiteral` is for labels).
    std::string literalText(const fluir::pt::Literal& v) {
      return std::visit(
        [](auto x) -> std::string {
          using T = std::decay_t<decltype(x)>;
          if constexpr (std::is_same_v<T, bool>)
            return x ? "true" : "false";
          else if constexpr (std::is_same_v<T, double>)
            return fmt::format("{:f}", x);
          else if constexpr (std::is_unsigned_v<T>)
            return fmt::format("{}", static_cast<unsigned long long>(x));
          else
            return fmt::format("{}", static_cast<long long>(x));
        },
        v);
    }

  }  // namespace

  void write(std::ostream& out, const fluir::pt::ParseTree& tree) { ParseTreeWriter(out).write(tree); }

  ParseTreeWriter::ParseTreeWriter(std::ostream& out) : out_(out) { }

  void ParseTreeWriter::write(const fluir::pt::ParseTree& tree) {
    doc_.Clear();
    doc_.InsertEndChild(doc_.NewDeclaration("xml version='1.0' encoding='UTF-8'"));

    Element* root = doc_.NewElement("fluir");
    doc_.InsertEndChild(root);

    header(root, tree.header);

    for (fluir::ID id : sortedIds(tree.declarations))
      declaration(root, tree.declarations.at(id));

    TwoSpacePrinter printer;
    doc_.Print(&printer);
    out_ << printer.CStr();
  }

  bool ParseTreeWriter::good() const { return out_.good(); }

  void ParseTreeWriter::header(Element* parent, const pt::Header& value) {
    Element* el = parent->InsertNewChildElement("header");
    version(el, value.version);
  }

  void ParseTreeWriter::version(Element* parent, const Version& value) {
    Element* el = parent->InsertNewChildElement("version");
    el->InsertNewChildElement("major")->SetText(int(value.major));
    el->InsertNewChildElement("minor")->SetText(int(value.minor));
    el->InsertNewChildElement("patch")->SetText(int(value.patch));
  }

  void ParseTreeWriter::declaration(Element* parent, const pt::Declaration& value) {
    std::visit(util::Overloaded{
                 [&](const pt::FunctionDecl& fn) { functionDecl(parent, fn); },
                 [&](const pt::Comment& c) { comment(parent, c); },
               },
               value);
  }

  void ParseTreeWriter::functionDecl(Element* parent, const pt::FunctionDecl& value) {
    Element* el = parent->InsertNewChildElement("function");
    el->SetAttribute("name", value.name.c_str());
    setId(el, value.id);
    setLocation(el, value.location);

    if (value.input.has_value() && !value.input->parameters.empty()) funcInputs(el, *value.input);
    if (value.output.has_value() && value.output->ret.has_value()) funcOutputs(el, *value.output);
    block(el, value.body);
  }

  void ParseTreeWriter::funcInputs(Element* parent, const pt::FunctionDecl::InputBlock& value) {
    Element* el = parent->InsertNewChildElement("input");
    for (const auto& param : value.parameters)
      funcParameter(el, param);
  }

  void ParseTreeWriter::funcParameter(Element* parent, const pt::FunctionDecl::Parameter& value) {
    Element* el = parent->InsertNewChildElement("param");
    el->SetAttribute("name", value.name.c_str());
    setId(el, value.id);
    if (!value.typeName.empty()) el->SetAttribute("type", value.typeName.c_str());
  }

  void ParseTreeWriter::funcOutputs(Element* parent, const pt::FunctionDecl::OutputBlock& value) {
    Element* el = parent->InsertNewChildElement("output");
    funcReturn(el, *value.ret);
  }

  void ParseTreeWriter::funcReturn(Element* parent, const pt::FunctionDecl::Return& value) {
    Element* el = parent->InsertNewChildElement("return");
    setId(el, value.id);
    if (!value.typeName.empty()) el->SetAttribute("type", value.typeName.c_str());
  }

  void ParseTreeWriter::block(Element* parent, const pt::Block& value) {
    blockContents(parent->InsertNewChildElement("body"), value);
  }

  void ParseTreeWriter::blockContents(Element* element, const pt::Block& value) {
    for (fluir::ID id : sortedIds(value.nodes))
      node(element, value.nodes.at(id));
    for (fluir::ID id : sortedIds(value.conduits))
      conduit(element, value.conduits.at(id));
  }

  void ParseTreeWriter::node(Element* parent, const pt::Node& value) {
    std::visit(util::Overloaded{
                 [&](const pt::Constant& n) { constant(parent, n); },
                 [&](const pt::Binary& n) { binary(parent, n); },
                 [&](const pt::Unary& n) { unary(parent, n); },
                 [&](const pt::Call& n) { call(parent, n); },
                 [&](const pt::Comment& n) { comment(parent, n); },
                 [&](const pt::Conditional& n) { conditional(parent, n); },
               },
               value);
  }

  void ParseTreeWriter::constant(Element* parent, const pt::Constant& value) {
    Element* el = parent->InsertNewChildElement("constant");
    setId(el, value.id);
    setLocation(el, value.location);
    literal(el, value.value);
  }

  void ParseTreeWriter::binary(Element* parent, const pt::Binary& value) {
    Element* el = parent->InsertNewChildElement("binary");
    setId(el, value.id);
    setLocation(el, value.location);
    el->SetAttribute("operator", std::string(fluir::stringify(value.op)).c_str());
  }

  void ParseTreeWriter::unary(Element* parent, const pt::Unary& value) {
    Element* el = parent->InsertNewChildElement("unary");
    setId(el, value.id);
    setLocation(el, value.location);
    el->SetAttribute("operator", std::string(fluir::stringify(value.op)).c_str());
  }

  void ParseTreeWriter::call(Element* parent, const pt::Call& value) {
    Element* el = parent->InsertNewChildElement("call");
    el->SetAttribute("target", value.target.c_str());
    setId(el, value.id);
    setLocation(el, value.location);
    if (value._return.has_value()) el->InsertNewChildElement("return");
    for (const auto& arg : value.arguments) {
      Element* ae = el->InsertNewChildElement("arg");
      ae->SetAttribute("name", arg.name.c_str());
    }
  }

  void ParseTreeWriter::conditional(Element* parent, const pt::Conditional& conditional) {
    auto* e = parent->InsertNewChildElement("conditional");
    setId(e, conditional.id);
    setLocation(e, conditional.location);
    blockPort(e, conditional.condition, "condition");
    {
      auto* input = e->InsertNewChildElement("input");
      auto sorted = sortedIds(conditional.inputs);
      for (const auto& id : sorted) {
        const auto& port = conditional.inputs.at(id);
        blockPort(input, port);
      }
    }
    {
      auto* output = e->InsertNewChildElement("output");
      auto sorted = sortedIds(conditional.outputs);
      for (const auto& id : sorted) {
        const auto& port = conditional.outputs.at(id);
        blockPort(output, port);
      }
    }
    // A branch has no id and no geometry: its contents sit straight inside its element.
    blockContents(e->InsertNewChildElement("then"), *conditional.thenScope);
    blockContents(e->InsertNewChildElement("else"), *conditional.elseScope);
  }

  void ParseTreeWriter::comment(Element* parent, const pt::Comment& value) {
    Element* el = parent->InsertNewChildElement("comment");
    setId(el, value.id);
    setLocation(el, value.location);
    el->SetText(value.text.c_str());
  }

  void ParseTreeWriter::blockPort(Element* parent, const pt::BlockPort& port, std::string_view name) {
    Element* e = parent->InsertNewChildElement(name.data());
    setIdReference(e, port.outerId, "outer"sv);
    setIdReference(e, port.innerId, "inner"sv);
    setInt(e, "y"sv, port.y);
  }

  void ParseTreeWriter::conduit(Element* parent, const pt::Conduit& value) {
    Element* el = parent->InsertNewChildElement("conduit");
    setId(el, value.id);
    el->SetAttribute("input", std::to_string(value.input).c_str());
    for (const auto& child : value.children)
      conduitOutput(el, child);
  }

  void ParseTreeWriter::conduitOutput(Element* parent, const pt::Conduit::Output& value) {
    Element* el = parent->InsertNewChildElement("output");
    el->SetAttribute("target", std::to_string(value.target).c_str());
    setInt(el, "index", value.index);
  }

  void ParseTreeWriter::literal(Element* parent, const pt::Literal& value) {
    Element* el = parent->InsertNewChildElement(std::string(literalTypeName(value)).c_str());
    el->SetText(literalText(value).c_str());
  }

  void ParseTreeWriter::setId(Element* element, ID id) { setIdReference(element, id, "id"sv); }

  void ParseTreeWriter::setIdReference(Element* element, ID id, std::string_view attribute) {
    element->SetAttribute(attribute.data(), std::to_string(id).c_str());
  }

  void ParseTreeWriter::setInt(Element* element, std::string_view name, int value) {
    element->SetAttribute(name.data(), std::to_string(value).c_str());
  }

  void ParseTreeWriter::setLocation(Element* element, const FlowGraphLocation& location) {
    setInt(element, "x", location.x);
    setInt(element, "y", location.y);
    setInt(element, "z", location.z);
    setInt(element, "w", location.width);
    setInt(element, "h", location.height);
  }

}  // namespace fluir::editor
