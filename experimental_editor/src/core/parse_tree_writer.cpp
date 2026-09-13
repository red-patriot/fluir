#include "editor/core/parse_tree_writer.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <tinyxml2.h>

#include "compiler/models/operator.hpp"

namespace fluir::editor {

  namespace {

    using tinyxml2::XMLDocument;
    using tinyxml2::XMLElement;

    template <typename... Fs>
    struct Overloaded : Fs... {
      using Fs::operator()...;
    };

    struct TwoSpacePrinter : tinyxml2::XMLPrinter {
      void PrintSpace(int depth) override {
        for (int i = 0; i < depth; ++i)
          Print("  ");
      }
    };

    template <typename Map>
    std::vector<fluir::ID> sortedIds(const Map& m) {
      std::vector<fluir::ID> ids;
      ids.reserve(m.size());
      for (const auto& kv : m)
        ids.push_back(kv.first);
      std::sort(ids.begin(), ids.end());
      return ids;
    }

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

    constexpr std::array<std::string_view, 10> LITERAL_TAGS{
      "f64", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "bool"};

    void setId(XMLElement* el, fluir::ID id) { el->SetAttribute("id", std::to_string(id).c_str()); }

    void setInt(XMLElement* el, const char* name, int value) { el->SetAttribute(name, std::to_string(value).c_str()); }

    void setLocation(XMLElement* el, const fluir::FlowGraphLocation& loc) {
      setInt(el, "x", loc.x);
      setInt(el, "y", loc.y);
      setInt(el, "z", loc.z);
      setInt(el, "w", loc.width);
      setInt(el, "h", loc.height);
    }

    void appendConstant(XMLElement* parent, const fluir::pt::Constant& node) {
      XMLElement* el = parent->InsertNewChildElement("constant");
      setId(el, node.id);
      setLocation(el, node.location);
      XMLElement* lit = el->InsertNewChildElement(LITERAL_TAGS[node.value.index()].data());
      lit->SetText(literalText(node.value).c_str());
    }

    void appendBinary(XMLElement* parent, const fluir::pt::Binary& node) {
      XMLElement* el = parent->InsertNewChildElement("binary");
      setId(el, node.id);
      setLocation(el, node.location);
      el->SetAttribute("operator", std::string(fluir::stringify(node.op)).c_str());
    }

    void appendUnary(XMLElement* parent, const fluir::pt::Unary& node) {
      XMLElement* el = parent->InsertNewChildElement("unary");
      setId(el, node.id);
      setLocation(el, node.location);
      el->SetAttribute("operator", std::string(fluir::stringify(node.op)).c_str());
    }

    void appendCall(XMLElement* parent, const fluir::pt::Call& node) {
      XMLElement* el = parent->InsertNewChildElement("call");
      el->SetAttribute("target", node.target.c_str());
      setId(el, node.id);
      setLocation(el, node.location);
      if (node._return.has_value()) {
        el->InsertNewChildElement("return");
      }
      for (const auto& arg : node.arguments) {
        XMLElement* ae = el->InsertNewChildElement("arg");
        ae->SetAttribute("name", arg.name.c_str());
      }
    }

    void appendComment(XMLElement* parent, const fluir::pt::Comment& comment) {
      XMLElement* el = parent->InsertNewChildElement("comment");
      setId(el, comment.id);
      setLocation(el, comment.location);
      el->SetText(comment.text.c_str());
    }

    void appendNode(XMLElement* parent, const fluir::pt::Node& node) {
      std::visit(
        [&](const auto& n) {
          using T = std::decay_t<decltype(n)>;
          if constexpr (std::is_same_v<T, fluir::pt::Constant>)
            appendConstant(parent, n);
          else if constexpr (std::is_same_v<T, fluir::pt::Binary>)
            appendBinary(parent, n);
          else if constexpr (std::is_same_v<T, fluir::pt::Unary>)
            appendUnary(parent, n);
          else if constexpr (std::is_same_v<T, fluir::pt::Call>)
            appendCall(parent, n);
          else if constexpr (std::is_same_v<T, fluir::pt::Comment>)
            appendComment(parent, n);
        },
        node);
    }

    void appendConduit(XMLElement* parent, const fluir::pt::Conduit& conduit) {
      XMLElement* el = parent->InsertNewChildElement("conduit");
      setId(el, conduit.id);
      el->SetAttribute("input", std::to_string(conduit.input).c_str());
      for (const auto& child : conduit.children) {
        XMLElement* oe = el->InsertNewChildElement("output");
        oe->SetAttribute("target", std::to_string(child.target).c_str());
        setInt(oe, "index", child.index);
      }
    }

    void appendInput(XMLElement* parent, const fluir::pt::FunctionDecl::InputBlock& input) {
      XMLElement* el = parent->InsertNewChildElement("input");
      for (const auto& param : input.parameters) {
        XMLElement* pe = el->InsertNewChildElement("param");
        pe->SetAttribute("name", param.name.c_str());
        setId(pe, param.id);
        if (!param.typeName.empty()) {
          pe->SetAttribute("type", param.typeName.c_str());
        }
      }
    }

    void appendOutput(XMLElement* parent, const fluir::pt::FunctionDecl::OutputBlock& output) {
      XMLElement* el = parent->InsertNewChildElement("output");
      const auto& ret = *output.ret;
      XMLElement* re = el->InsertNewChildElement("return");
      setId(re, ret.id);
      if (!ret.typeName.empty()) {
        re->SetAttribute("type", ret.typeName.c_str());
      }
    }

    void appendBlock(XMLElement* parent, const fluir::pt::Block& body) {
      XMLElement* el = parent->InsertNewChildElement("body");
      for (fluir::ID id : sortedIds(body.nodes))
        appendNode(el, body.nodes.at(id));
      for (fluir::ID id : sortedIds(body.conduits))
        appendConduit(el, body.conduits.at(id));
    }

    void appendFunction(XMLElement* parent, const fluir::pt::FunctionDecl& fn) {
      XMLElement* el = parent->InsertNewChildElement("function");
      el->SetAttribute("name", fn.name.c_str());
      setId(el, fn.id);
      setLocation(el, fn.location);

      if (fn.input.has_value() && !fn.input->parameters.empty()) {
        appendInput(el, *fn.input);
      }
      if (fn.output.has_value() && fn.output->ret.has_value()) {
        appendOutput(el, *fn.output);
      }
      appendBlock(el, fn.body);
    }

  }  // namespace

  ParseTreeWriter::ParseTreeWriter(std::ostream& out) : out_(out) { }

  void ParseTreeWriter::write(const fluir::pt::ParseTree& tree) {
    XMLDocument doc;
    doc.InsertEndChild(doc.NewDeclaration("xml version='1.0' encoding='UTF-8'"));

    XMLElement* root = doc.NewElement("fluir");
    doc.InsertEndChild(root);

    XMLElement* header = root->InsertNewChildElement("header");
    XMLElement* version = header->InsertNewChildElement("version");
    version->InsertNewChildElement("major")->SetText(int(tree.header.version.major));
    version->InsertNewChildElement("minor")->SetText(int(tree.header.version.minor));
    version->InsertNewChildElement("patch")->SetText(int(tree.header.version.patch));

    for (fluir::ID id : sortedIds(tree.declarations)) {
      std::visit(Overloaded{
                   [&](const fluir::pt::FunctionDecl& fn) { appendFunction(root, fn); },
                   [&](const fluir::pt::Comment& comment) { appendComment(root, comment); },
                 },
                 tree.declarations.at(id));
    }

    TwoSpacePrinter printer;
    doc.Print(&printer);
    out_ << printer.CStr();
  }

  bool ParseTreeWriter::good() const { return out_.good(); }

}  // namespace fluir::editor
