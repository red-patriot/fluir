#include "compiler/frontend/parser.hpp"

#include <fstream>
#include <optional>
#include <sstream>

#include <fmt/format.h>

using namespace std::string_literals;

namespace fluir {
  template <typename... FmtArgs>
  void Parser::panicIf(bool condition, Element* element, std::string_view format, FmtArgs... args) {
    if (condition) {
      panicAt(element, format, args...);
    }
  }
  template <typename... FmtArgs>
  [[noreturn]] void Parser::panicAt(Element* element, std::string_view format, FmtArgs... args) {
    ctx_.diagnostics.emitError(fmt::vformat(format, fmt::make_format_args(args...)),
                               std::make_shared<SourceLocation>(element->GetLineNum(), filename_));
    throw PanicMode{};
  }

  Results<pt::ParseTree> parseFile(Context& ctx, const std::filesystem::path& source) {
    Parser parser{ctx};
    return parser.parseFile(source);
  }

  Results<pt::ParseTree> parseString(Context& ctx, const std::string_view source) {
    Parser parser{ctx};
    return parser.parseString(source);
  }

  Parser::Parser(Context& ctx) : ctx_(ctx) { }

  Results<pt::ParseTree> Parser::parseFile(const std::filesystem::path& file) {
    filename_ = file.filename().string();
    doc_.Clear();
    std::ifstream fin(file);
    std::stringstream ss;
    ss << fin.rdbuf();

    doc_.Parse(ss.str().c_str());

    flowGraph();

    if (ctx_.diagnostics.containsErrors()) {
      return NoResult;
    } else {
      return std::move(tree_);
    }
  }

  Results<pt::ParseTree> Parser::parseString(const std::string_view source) {
    filename_ = "<FROM STRING>";
    doc_.Clear();
    doc_.Parse(source.data());

    flowGraph();

    if (ctx_.diagnostics.containsErrors()) {
      return NoResult;
    } else {
      return std::move(tree_);
    }
  }

  void Parser::flowGraph() {
    std::string_view expectedRoot = "fluir";
    try {
      auto root = doc_.RootElement();

      panicIf(root->Name() != expectedRoot,
              root,
              "Expected root element to be '{}', found '{}'.",
              expectedRoot,
              root->Name());
      // TODO: Check metadata

      for (auto child = root->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
        declaration(child);
      }
    } catch (const PanicMode&) {
      // If something goes wrong at this level, there isn't really anything to
      // do except bail
      return;
    }
  }

  void Parser::declaration(Element* element) {
    std::string_view name = element->Name();
    try {
      // TODO: This could use a trie
      if (name == "function") {
        functionDecl(element);
      } else {
        panicAt(element, "Unexpected element '{}'. Expected declaration.", name);
      }
    } catch (const PanicMode&) {
      // Synchronize here
    }
  }

  void Parser::functionDecl(Element* element) {
    constexpr std::string_view type = "function";
    std::string_view name = getAttribute(element, type, "name");
    ID id = parseId(element, type);
    auto location = parseLocation(element, type);

    auto bodyElement = element->FirstChildElement("body");
    panicIf(!bodyElement, element, "Function '{}' has no body. Expected a '<body>' element.", name);

    pt::Block body = block(bodyElement->FirstChildElement());

    panicIf(tree_.declarations.contains(id),
            element,
            "Duplicate declaration IDs. Function '{}' has id {}, but that ID is already in use.",
            name,
            id);
    tree_.declarations.emplace(id, pt::FunctionDecl{id, location, std::string(name), body});
  }

  pt::Block Parser::block(Element* element) {
    auto block = pt::EMPTY_BLOCK;
    for (; element != nullptr; element = element->NextSiblingElement()) {
      try {
        if (element->Name() == "conduit"s) {
          // Parse a conduit
          auto result = conduit(element);
          auto& [id, resultConduit] = result;
          panicIf(block.nodes.contains(id) || block.conduits.contains(id),
                  element,
                  "Duplicate IDs. Conduit has ID {}, but that ID is already in use.",
                  id);
          block.conduits.emplace(std::move(result));

        } else {
          // Parse any other node
          auto result = node(element);
          auto& [id, resultNode] = result;
          panicIf(block.nodes.contains(id) || block.conduits.contains(id),
                  element,
                  "Duplicate IDs. Node <{}> has ID {}, but that ID is already in use.",
                  element->Name(),
                  id);
          block.nodes.emplace(std::move(result));
        }
      } catch (const PanicMode&) {
        // Synchronize here
        continue;
      }
    }
    return block;
  }

  std::pair<ID, pt::Node> Parser::node(Element* element) {
    // TODO: This could use a trie
    std::string_view type = element->Name();
    if (type == "constant") {
      return constant(element);
    } else if (type == "binary") {
      return binary(element);
    } else if (type == "unary") {
      return unary(element);
    } else {
      panicAt(element, "Unexpected element '{}'. Expected a node.", type);
    }
  }

  std::pair<ID, pt::Conduit> Parser::conduit(Element* element) {
    constexpr std::string_view type = "conduit";
    auto id = parseId(element, type);
    auto input = parseIdReference(element, "input", type);
    auto indexStr = getOptionalAttribute(element, "index", "0");
    auto index = std::stoi(indexStr.data());
    std::vector<pt::Conduit::Output> children;
    for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
      children.push_back(conduitOutput(child));
    }

    return {id, pt::Conduit{.id = id, .input = input, .index = index, .children = children}};
  }

  pt::Conduit::Output Parser::conduitOutput(Element* element) {
    constexpr std::string_view type = "output";
    auto target = parseIdReference(element, "target", type);
    auto indexStr = getOptionalAttribute(element, "index", "0");
    auto index = std::stoi(indexStr.data());

    return pt::Conduit::Output{.target = target, .index = index};
  }

  std::pair<ID, pt::Node> Parser::constant(Element* element) {
    std::string_view type = "constant";
    auto id = parseId(element, type);
    auto location = parseLocation(element, type);
    auto value = literal(element->FirstChildElement());

    return {id, pt::Constant{id, location, value}};
  }

  std::pair<ID, pt::Node> Parser::binary(Element* element) {
    std::string_view type = "binary";
    auto id = parseId(element, type);
    auto location = parseLocation(element, type);
    // TODO: Remove this
    auto lhs = parseOptionalIdReference(element, "lhs", type);
    auto rhs = parseOptionalIdReference(element, "rhs", type);
    auto op = parseOperator(element, "operator", type);

    return {id, pt::Binary{id, location, lhs, rhs, op}};
  }

  std::pair<ID, pt::Node> Parser::unary(Element* element) {
    std::string_view type = "unary";
    auto id = parseId(element, type);
    auto location = parseLocation(element, type);
    // TODO: Remove this
    auto lhs = parseOptionalIdReference(element, "lhs", type);
    auto op = parseOperator(element, "operator", type);

    return {id, pt::Unary{id, location, lhs, op}};
  }

  pt::Literal Parser::literal(Element* element) {
    // TODO: This could use a trie to be faster
    // TODO: Support other literal types
    std::string_view name = element->Name();
    if (name == "float") {
      return fl_float(element);
    } else {
      // TODO: Error
      throw PanicMode{};
    }
  }
  pt::Float Parser::fl_float(Element* element) {
    double value = 0.0;
    auto error = element->QueryDoubleText(&value);
    panicIf(error != tinyxml2::XML_SUCCESS,
            element,
            "Expected a numeric value in element '<{}>'. '{}' cannot be parsed as a number.",
            "float",
            element->GetText());
    return value;
  }

  std::string_view Parser::getAttribute(Element* element, std::string_view type, std::string_view attribute) {
    auto value = element->Attribute(attribute.data());
    panicIf(value == nullptr, element, "{} element is missing attribute '{}'.", type, attribute);

    return value;
  }

  std::string_view Parser::getOptionalAttribute(Element* element,
                                                std::string_view attribute,
                                                std::string_view defaultValue) {
    auto value = element->Attribute(attribute.data());
    if (value == nullptr) {
      return defaultValue;
    } else {
      return value;
    }
  }

  ID Parser::parseId(Element* element, std::string_view type) { return parseIdReference(element, "id", type); }

  ID Parser::parseIdReference(Element* element, std::string_view attribute, std::string_view type) {
    ID reference = INVALID_ID;
    auto error = element->QueryUnsigned64Attribute(attribute.data(), &reference);
    panicIf(error != tinyxml2::XML_SUCCESS, element, "{} element is missing attribute '{}'.", type, attribute);

    return reference;
  }

  ID Parser::parseOptionalIdReference(Element* element, std::string_view attribute, std::string_view) {
    ID reference = INVALID_ID;
    element->QueryUnsigned64Attribute(attribute.data(), &reference);

    return reference;
  }

  FlowGraphLocation Parser::parseLocation(Element* element, std::string_view type) {
    return {
      .x = std::atoi(getAttribute(element, type, "x").data()),
      .y = std::atoi(getAttribute(element, type, "y").data()),
      .z = std::atoi(getAttribute(element, type, "z").data()),
      .width = std::atoi(getAttribute(element, type, "w").data()),
      .height = std::atoi(getAttribute(element, type, "h").data()),
    };
  }

  Operator Parser::parseOperator(Element* element, std::string_view attribute, std::string_view type) {
    std::string_view opText = element->Attribute(attribute.data());
    // TODO: This could be made faster...
    if (opText == "+") {
      return Operator::PLUS;
    } else if (opText == "-") {
      return Operator::MINUS;
    } else if (opText == "*") {
      return Operator::STAR;
    } else if (opText == "/") {
      return Operator::SLASH;
    } else {
      panicAt(element, "Unrecognized operator '{}' in element '<{}>'.", opText, type);
    }
  }

  std::string Parser::SourceLocation::str() const { return fmt::format("on line {} of '{}'", lineNo, filename); }
}  // namespace fluir
