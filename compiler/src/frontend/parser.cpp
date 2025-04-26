#include "compiler/frontend/parser.hpp"

#include <fstream>
#include <sstream>

#include <fmt/format.h>

using namespace std::string_literals;

namespace fluir {
  template <typename... FmtArgs>
  void Parser::panicIf(bool condition, Element* element,
                       std::string_view format,
                       FmtArgs&&... args) {
    if (condition) {
      panicAt(element, format, std::forward<FmtArgs>(args)...);
    }
  }
  template <typename... FmtArgs>
  [[noreturn]] void Parser::panicAt(Element* element,
                                    std::string_view format,
                                    FmtArgs&&... args) {
    diagnostics_.emitError(
        fmt::vformat(format, fmt::make_format_args(std::forward<FmtArgs>(args)...)),
        std::make_shared<SourceLocation>(element->GetLineNum(), filename_));
    throw PanicMode{};
  }

  Results<pt::ParseTree> parseFile(const std::filesystem::path& source) {
    Parser parser{};
    return parser.parseFile(source);
  }

  Results<pt::ParseTree> parseString(const std::string_view source) {
    Parser parser{};
    return parser.parseString(source);
  }

  Results<pt::ParseTree> Parser::parseFile(const std::filesystem::path& file) {
    filename_ = file.filename();
    doc_.Clear();
    std::ifstream fin(file);
    std::stringstream ss;
    ss << fin.rdbuf();

    doc_.Parse(ss.str().c_str());

    flowGraph();

    if (diagnostics_.containsErrors()) {
      return Results<pt::ParseTree>{std::move(diagnostics_)};
    } else {
      return {std::move(tree_),
              std::move(diagnostics_)};
    }
  }

  Results<pt::ParseTree> Parser::parseString(const std::string_view source) {
    filename_ = "<FROM STRING>";
    doc_.Clear();
    doc_.Parse(source.data());

    flowGraph();

    if (diagnostics_.containsErrors()) {
      return Results<pt::ParseTree>{std::move(diagnostics_)};
    } else {
      return {std::move(tree_),
              std::move(diagnostics_)};
    }
  }

  void Parser::flowGraph() {
    std::string_view expectedRoot = "fluir";
    try {
      auto root = doc_.RootElement();

      panicIf(root->Name() != expectedRoot,
              root, "Expected root element to be '{}', found '{}'.",
              expectedRoot, root->Name());
      // TODO: Check metadata

      for (auto child = root->FirstChildElement();
           child != nullptr;
           child = child->NextSiblingElement()) {
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
      if (name == "fl:function") {
        functionDecl(element);
      } else {
        panicAt(element, "Unexpected element '{}'. Expected declaration.", name);
      }
    } catch (const PanicMode&) {
      // Synchronize here
    }
  }

  void Parser::functionDecl(Element* element) {
    constexpr std::string_view type = "fl:function";
    std::string_view name = getAttribute(element, type, "name");
    ID id = parseId(element, type);
    auto location = parseLocation(element, type);

    auto bodyElement = element->FirstChildElement("body");
    panicIf(!bodyElement, element,
            "Function '{}' has no body. Expected a '<body>' element.", name);

    pt::Block body = block(bodyElement->FirstChildElement());

    panicIf(tree_.declarations.contains(id),
            element,
            "Duplicate declaration IDs. Function '{}' has id {}, but that ID is already in use.",
            name, id);
    tree_.declarations.emplace(
        id,
        pt::FunctionDecl{id, location, std::string(name), body});
  }

  pt::Block Parser::block(Element* element) {
    auto block = pt::EMPTY_BLOCK;
    for (; element != nullptr; element = element->NextSiblingElement()) {
      try {
        auto resultNode = node(element);
        // TODO: Check for duplicates
        if (!block.contains(resultNode.first)) {
          block.emplace(std::move(resultNode));
        } else {
          panicAt(element,
                  "Duplicate node IDs. Node <{}> has ID {}, but that ID is already in use.",
                  element->Name(), resultNode.first);
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
    if (type == "fl:constant") {
      return constant(element);
    } else if (type == "fl:binary") {
      return binary(element);
    } else if (type == "fl:unary") {
      return unary(element);
    } else {
      panicAt(element,
              "Unexpected element '{}'. Expected a node.",
              type);
    }
  }

  std::pair<ID, pt::Node> Parser::constant(Element* element) {
    std::string_view type = "fl:constant";
    auto id = parseId(element, type);
    auto location = parseLocation(element, type);
    auto value = literal(element->FirstChildElement());

    return {id, pt::Constant{id, location, value}};
  }

  std::pair<ID, pt::Node> Parser::binary(Element* element) {
    std::string_view type = "fl:binary";
    auto id = parseId(element, type);
    auto location = parseLocation(element, type);
    auto lhs = parseIdReference(element, "lhs", type);
    auto rhs = parseIdReference(element, "rhs", type);
    auto op = parseOperator(element, "operator", type);

    return {id, pt::Binary{id, location, lhs, rhs, op}};
  }

  std::pair<ID, pt::Node> Parser::unary(Element* element) {
    std::string_view type = "fl:unary";
    auto id = parseId(element, type);
    auto location = parseLocation(element, type);
    auto lhs = parseIdReference(element, "lhs", type);
    auto op = parseOperator(element, "operator", type);

    return {id, pt::Unary{id, location, lhs, op}};
  }

  pt::Literal Parser::literal(Element* element) {
    // TODO: This could use a trie to be faster
    // TODO: Support other literal types
    std::string_view name = element->Name();
    if (name == "fl:float") {
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
            "fl:float", element->GetText());
    return value;
  }

  std::string_view Parser::getAttribute(Element* element,
                                        std::string_view type,
                                        std::string_view attribute) {
    auto value = element->Attribute(attribute.data());
    panicIf(value == nullptr,
            element,
            "{} element is missing attribute '{}'.",
            type, attribute);

    return value;
  }

  ID Parser::parseId(Element* element, std::string_view type) {
    return parseIdReference(element, "id", type);
  }

  ID Parser::parseIdReference(Element* element,
                              std::string_view attribute,
                              std::string_view type) {
    ID reference = INVALID_ID;
    auto error = element->QueryUnsigned64Attribute(attribute.data(), &reference);
    panicIf(error != tinyxml2::XML_SUCCESS,
            element,
            "{} element is missing attribute '{}'.", type, attribute);

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

  Operator Parser::parseOperator(Element* element,
                                 std::string_view attribute,
                                 std::string_view type) {
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
      panicAt(element,
              "Unrecognized operator '{}' in element '<{}>'.", opText, type);
    }
  }

  std::string Parser::SourceLocation::str() const {
    return fmt::format("on line {} of '{}'", lineNo, filename);
  }
}  // namespace fluir
