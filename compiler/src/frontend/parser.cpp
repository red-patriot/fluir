#include "compiler/frontend/parser.hpp"

#include <fstream>
#include <optional>
#include <sstream>

#include <fmt/format.h>

#include "compiler/frontend/text_parse.hpp"

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
    if (name == "f64") {
      return f64(element);
    } else if (name == "i8") {
      return i8(element);
    } else if (name == "i16") {
      return i16(element);
    } else if (name == "i32") {
      return i32(element);
    } else if (name == "i64") {
      return i64(element);
    } else if (name == "u8") {
      return u8(element);
    } else if (name == "u16") {
      return u16(element);
    } else if (name == "u32") {
      return u32(element);
    } else if (name == "u64") {
      return u64(element);
    }
    panicAt(
      element, "Expected a literal element. Found '<{}>', which is not a recognized literal type.", element->Name());
  }
  pt::F64 Parser::f64(Element* element) {
    double value = 0.0;
    auto error = element->QueryDoubleText(&value);
    panicIf(error != tinyxml2::XML_SUCCESS,
            element,
            "Expected a numeric value in element '<{}>'. '{}' cannot be parsed as a number.",
            "f64",
            element->GetText());
    return value;
  }

  pt::I8 Parser::i8(Element* element) {
    auto value = fe::parseNumber<pt::I8>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            "Expected an integer value in element '<{}>'. '{}' cannot be parsed as an integer.",
            "i8",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            "The literal '{}' is outside the range of {}.",
            element->GetText(),
            "i8");
    ctx_.diagnostics.emitInternalError("Control reached an impossible point");
    return {};
  }
  pt::I16 Parser::i16(Element* element) {
    auto value = fe::parseNumber<pt::I16>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            "Expected an integer value in element '<{}>'. '{}' cannot be parsed as an integer.",
            "i16",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            "The literal '{}' is outside the range of {}.",
            element->GetText(),
            "i16");
    ctx_.diagnostics.emitInternalError("Control reached an impossible point");
    return {};
  }
  pt::I32 Parser::i32(Element* element) {
    auto value = fe::parseNumber<pt::I32>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            "Expected an integer value in element '<{}>'. '{}' cannot be parsed as an integer.",
            "i32",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            "The literal '{}' is outside the range of {}.",
            element->GetText(),
            "i32");
    ctx_.diagnostics.emitInternalError("Control reached an impossible point");
    return {};
  }
  pt::I64 Parser::i64(Element* element) {
    auto value = fe::parseNumber<pt::I64>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            "Expected an integer value in element '<{}>'. '{}' cannot be parsed as an integer.",
            "i64",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            "The literal '{}' is outside the range of {}.",
            element->GetText(),
            "i64");
    ctx_.diagnostics.emitInternalError("Control reached an impossible point");
    return {};
  }

  pt::U8 Parser::u8(Element* element) {
    auto value = fe::parseNumber<pt::U8>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            "Expected an unsigned integer value in element '<{}>'. '{}' cannot be parsed as an unsigned integer.",
            "u8",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            "The literal '{}' is outside the range of {}.",
            element->GetText(),
            "u8");
    ctx_.diagnostics.emitInternalError("Control reached an impossible point");
    return {};
  }
  pt::U16 Parser::u16(Element* element) {
    auto value = fe::parseNumber<pt::U16>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            "Expected an unsigned integer value in element '<{}>'. '{}' cannot be parsed as an unsigned integer.",
            "u16",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            "The literal '{}' is outside the range of {}.",
            element->GetText(),
            "u16");
    ctx_.diagnostics.emitInternalError("Control reached an impossible point");
    return {};
  }
  pt::U32 Parser::u32(Element* element) {
    auto value = fe::parseNumber<pt::U32>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            "Expected an unsigned integer value in element '<{}>'. '{}' cannot be parsed as an unsigned integer.",
            "u32",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            "The literal '{}' is outside the range of {}.",
            element->GetText(),
            "u32");
    ctx_.diagnostics.emitInternalError("Control reached an impossible point");
    return {};
  }
  pt::U64 Parser::u64(Element* element) {
    auto value = fe::parseNumber<pt::U64>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            "Expected an unsigned integer value in element '<{}>'. '{}' cannot be parsed as an unsigned integer.",
            "u64",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            "The literal '{}' is outside the range of {}.",
            element->GetText(),
            "u64");
    ctx_.diagnostics.emitInternalError("Control reached an impossible point");
    return {};
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
