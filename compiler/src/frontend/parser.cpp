#include "compiler/frontend/parser.hpp"

#include <fstream>
#include <optional>
#include <sstream>

#include <fmt/format.h>

#include "compiler/frontend/text_parse.hpp"
#include "fluir/util/trie.hpp"

using namespace std::string_literals;

namespace fluir {
  template <typename... FmtArgs>
  void Parser::panicAt(Element* element,
                       diagnostic::Code code,
                       fmt::format_string<FmtArgs...> format,
                       FmtArgs&&... args) {
    if (diagnostic::isError(code)) {
      failed = true;
    }
    ctx_.diagnosticSink.emitAtLine(
      code, ctx_.currentFile, element ? element->GetLineNum() : 0, format, std::forward<FmtArgs>(args)...);
  }

  template <typename... FmtArgs>
  void Parser::panicIf(
    bool condition, Element* element, diagnostic::Code code, fmt::format_string<FmtArgs...> format, FmtArgs&&... args) {
    if (condition) {
      panicAt(element, code, format, std::forward<FmtArgs>(args)...);
    }
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
    try {
      if (!std::filesystem::exists(file)) {
        ctx_.diagnosticSink.emitAtLine(diagnostic::Code::ERROR_FILE_DOES_NOT_EXIST, file, 0);
      }
    } catch (diagnostic::Panic) {
      return NoResult;
    }

    ctx_.currentFile = file;
    doc_.Clear();
    std::ifstream fin(file);
    std::stringstream ss;
    ss << fin.rdbuf();

    doc_.Parse(ss.str().c_str());

    if (!flowGraph()) {
      return NoResult;
    }

    return std::move(tree_);
  }

  Results<pt::ParseTree> Parser::parseString(const std::string_view source) {
    ctx_.currentFile = "<FROM STRING>";
    doc_.Clear();
    doc_.Parse(source.data());

    if (!flowGraph()) {
      return NoResult;
    }
    return std::move(tree_);
  }

  bool Parser::flowGraph() {
    std::string_view expectedRoot = "fluir";
    try {
      auto root = doc_.RootElement();

      panicIf(root->Name() != expectedRoot,
              root,
              diagnostic::Code::ERROR_WRONG_ROOT_ELEMENT,
              "Expected root element to be '{}', found '{}'.",
              expectedRoot,
              root->Name());
      bool headerFound = false;
      for (auto child = root->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
        std::string_view name = child->Name();
        if (name == "header") {
          panicIf(headerFound, child, diagnostic::Code::ERROR_UNEXPECTED_DUPLICATE_HEADER);
          header(child);
          headerFound = true;
          continue;
        }

        declaration(child);
      }
      panicIf(!headerFound, doc_.RootElement(), diagnostic::Code::ERROR_MISSING_MODULE_HEADER);
    } catch (const diagnostic::Panic&) {
      // If something goes wrong at this level, there isn't really anything to
      // do except bail
      return false;
    }
    return !failed;
  }

  void Parser::header(Element* element) {
    constexpr std::string_view version_tag = "version";
    bool versionFound = false;
    for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
      std::string_view name = child->Name();
      if (name == version_tag) {
        tree_.header.version = version(child);
        if (!ctx_.ignoreVersionChecks) {
          panicIf(tree_.header.version != ctx_.version,
                  nullptr,
                  diagnostic::Code::ERROR_INCORRECT_MODULE_VERSION,
                  "Module version {}.{}.{}, which cannot be compiled by this version of the compiler.",
                  tree_.header.version.major,
                  tree_.header.version.minor,
                  tree_.header.version.patch);
        }
        versionFound = true;
      } else {
        panicAt(child, diagnostic::Code::ERROR_UNEXPECTED_ELEMENT, "{} is invalid in program header", name);
      }
    }
    panicIf(
      !versionFound, element, diagnostic::Code::ERROR_MISSING_ELEMENT, "No version element in the program header.");
  }

  Version Parser::version(Element* element) {
    Version version{0, 0, 0};

    for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
      if (const std::string_view name = child->Name(); name == "major") {
        auto major = fe::parseInteger(child->GetText());
        panicIf(!major.has_value(),
                child,
                diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
                "Expected a version number, got '{}'.",
                child->GetText());
        version.major = static_cast<uint8_t>(major.value());
      } else if (name == "minor") {
        auto major = fe::parseInteger(child->GetText());
        panicIf(!major.has_value(),
                child,
                diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
                "Expected a version number, got '{}'.",
                child->GetText());
        version.minor = static_cast<uint8_t>(major.value());
      } else if (name == "patch") {
        auto patch = fe::parseInteger(child->GetText());
        panicIf(!patch.has_value(),
                child,
                diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
                "Expected a version number, got '{}'.",
                child->GetText());
        version.patch = static_cast<uint8_t>(patch.value());
      }
    }

    return version;
  }

  void Parser::declaration(Element* element) {
    static const util::Trie<void (*)(Parser* p, Element* e)> declarationParsers{
      [](Parser* p, Element* e) -> void {
        p->panicAt(e, diagnostic::Code::ERROR_UNEXPECTED_ELEMENT, "Expected a declaration.");
      },
      {{"function", [](Parser* p, Element* e) -> void { p->functionDecl(e); }}}};

    std::string_view name = element->Name();
    FLUIR_SYNCHRONIZE_PANIC(ctx_.diag) {
      auto declarationParser = declarationParsers.at(name);
      declarationParser(this, element);
    };
  }

  void Parser::functionDecl(Element* element) {
    constexpr std::string_view bodyTag = "body";
    constexpr std::string_view inputTag = "input";
    constexpr std::string_view outputTag = "output";
    std::string_view name = getAttribute(element, "name");
    ID id = parseId(element);
    auto location = parseLocation(element);
    std::optional<pt::Block> body;
    pt::FunctionDecl::Parameters parameters;
    pt::FunctionDecl::Returns returns;
    for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
      std::string_view childName = child->Name();
      if (childName == bodyTag) {
        body = block(child);
      } else if (childName == inputTag) {
        parameters = funcInputs(child);
      } else if (childName == outputTag) {
        returns = funcOutputs(child);
      } else {
        panicAt(child, diagnostic::Code::ERROR_UNEXPECTED_ELEMENT, "Unexpected element '{}'.", childName);
      }
    }

    panicIf(!body, element, diagnostic::Code::ERROR_MISSING_ELEMENT, "Expected a '<body>' element.");

    panicIf(tree_.declarations.contains(id), element, diagnostic::Code::ERROR_DUPLICATE_IDS_FOUND);
    tree_.declarations.emplace(
      id,
      pt::FunctionDecl{id, location, std::string(name), std::move(*body), std::move(parameters), std::move(returns)});
  }

  pt::FunctionDecl::Parameters Parser::funcInputs(Element* section) {
    static constexpr std::string_view paramTag = "param";
    pt::FunctionDecl::Parameters parameters;

    for (auto child = section->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
      FLUIR_SYNCHRONIZE_PANIC(ctx_.diag) {
        panicIf(child->Name() != paramTag,
                child,
                diagnostic::Code::ERROR_UNEXPECTED_ELEMENT,
                "Unexpected element <{}>. Expected <{}>",
                child->Name(),
                paramTag);
        auto result = funcParameter(child);
        auto& [id, param] = result;
        parameters.insert({id, param});
      };
    }

    return parameters;
  }

  WithID<pt::FunctionDecl::Parameter> Parser::funcParameter(Element* element) {
    auto name = getAttribute(element, "name");
    auto id = parseId(element);
    auto location = parseBorderingLocation(element);
    auto typeName = getAttribute(element, "type");

    return {id,
            pt::FunctionDecl::Parameter{
              .id = id, .location = location, .name = std::string(name), .typeName = std::string(typeName)}};
  }

  pt::FunctionDecl::Returns Parser::funcOutputs(Element* section) {
    static constexpr std::string_view returnTag = "return";
    pt::FunctionDecl::Returns ret;

    for (auto child = section->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
      FLUIR_SYNCHRONIZE_PANIC(ctx_.diag) {
        panicIf(child->Name() != returnTag,
                child,
                diagnostic::Code::ERROR_UNEXPECTED_ELEMENT,
                "Unexpected element <{}>. Expected <{}>",
                child->Name(),
                returnTag);
        auto result = funcReturn(child);
        ret.insert(result);
      };
    }

    return ret;
  }

  WithID<pt::FunctionDecl::Return> Parser::funcReturn(Element* element) {
    auto id = parseId(element);
    auto location = parseBorderingLocation(element);
    auto typeName = getAttribute(element, "type");
    return {id, pt::FunctionDecl::Return{.id = id, .location = location, .typeName = std::string(typeName)}};
  }

  pt::Block Parser::block(Element* body) {
    auto block = pt::EMPTY_BLOCK;
    for (auto child = body->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
      FLUIR_SYNCHRONIZE_PANIC(ctx_.diag) {
        if (child->Name() == "conduit"s) {
          // Parse a conduit
          auto result = conduit(child);
          auto& [id, resultConduit] = result;
          panicIf(block.nodes.contains(id) || block.conduits.contains(id),
                  child,
                  diagnostic::Code::ERROR_DUPLICATE_IDS_FOUND);
          block.conduits.emplace(std::move(result));

        } else {
          // Parse any other node
          auto result = node(child);
          auto& [id, resultNode] = result;
          panicIf(block.nodes.contains(id) || block.conduits.contains(id),
                  child,
                  diagnostic::Code::ERROR_DUPLICATE_IDS_FOUND);
          block.nodes.emplace(std::move(result));
        }
      };
    }
    return block;
  }

  WithID<pt::Node> Parser::node(Element* element) {
    using NodeIdPair = WithID<pt::Node>;
    static const util::Trie<NodeIdPair (*)(Parser* p, Element* e)> nodeParsers{
      [](Parser* p, Element* e) -> NodeIdPair {
        p->panicAt(e, diagnostic::Code::ERROR_UNEXPECTED_ELEMENT, "Expected a node.");
        std::unreachable();
      },
      {{"constant", [](Parser* p, Element* e) -> NodeIdPair { return p->constant(e); }},
       {"binary", [](Parser* p, Element* e) -> NodeIdPair { return p->binary(e); }},
       {"unary", [](Parser* p, Element* e) -> NodeIdPair { return p->unary(e); }},
       {"call", [](Parser* p, Element* e) -> NodeIdPair { return p->call(e); }}}};

    std::string_view type = element->Name();
    auto nodeParser = nodeParsers.at(type);
    return nodeParser(this, element);
  }

  WithID<pt::Conduit> Parser::conduit(Element* element) {
    const auto id = parseId(element);
    const auto input = parseIdReference(element, "input");
    const auto indexStr = getOptionalAttribute(element, "index", "0");
    const auto index = std::stoi(indexStr.data());
    std::vector<pt::Conduit::Output> children;
    for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
      children.push_back(conduitOutput(child));
    }

    return {id, pt::Conduit{.id = id, .input = input, .index = index, .children = children}};
  }

  pt::Conduit::Output Parser::conduitOutput(Element* element) {
    auto target = parseIdReference(element, "target");
    auto indexStr = getOptionalAttribute(element, "index", "0");
    auto index = std::stoi(indexStr.data());

    return pt::Conduit::Output{.target = target, .index = index};
  }

  WithID<pt::Node> Parser::constant(Element* element) {
    auto id = parseId(element);
    auto location = parseLocation(element);
    auto value = literal(element->FirstChildElement());

    return {id, pt::Constant{id, location, value}};
  }

  WithID<pt::Node> Parser::binary(Element* element) {
    auto id = parseId(element);
    auto location = parseLocation(element);
    // TODO: Remove this
    auto lhs = parseOptionalIdReference(element, "lhs");
    auto rhs = parseOptionalIdReference(element, "rhs");
    auto op = parseOperator(element, "operator");

    return {id, pt::Binary{id, location, lhs, rhs, op}};
  }

  WithID<pt::Node> Parser::unary(Element* element) {
    auto id = parseId(element);
    auto location = parseLocation(element);
    // TODO: Remove this
    auto lhs = parseOptionalIdReference(element, "lhs");
    auto op = parseOperator(element, "operator");

    return {id, pt::Unary{id, location, lhs, op}};
  }

  WithID<pt::Node> Parser::call(Element* element) {
    auto id = parseId(element);
    auto location = parseLocation(element);
    auto target = getAttribute(element, "target");
    pt::Call::Arguments arguments;
    std::optional<pt::Call::Return> return_{std::nullopt};
    for (auto child = element->FirstChildElement(); child != nullptr; child = child->NextSiblingElement()) {
      std::string_view childName = child->Name();
      if (childName == "return") {
        [[maybe_unused]] auto index = getAttribute(child, "index");
        return_ = pt::Call::Return{};
      } else if (childName == "arg") {
        auto name = getAttribute(child, "name");
        auto indexStr = getAttribute(child, "index");
        auto index = fe::parseNumber<int>(indexStr);
        // TODO: Error checking
        arguments.push_back(pt::Call::Argument{.name = std::string(name), .index = index.value()});
      }
    }

    return {
      id,
      pt::Call{
        .id = id, .location = location, .target = std::string(target), ._return = return_, .arguments = arguments}};
  }

  pt::Literal Parser::literal(Element* element) {
    static const util::Trie<pt::Literal (*)(Parser*, Element*)> literalParsers{
      // TODO: Support other literal types
      [](Parser* self, Element* element) -> pt::Literal {
        self->panicAt(
          element, diagnostic::Code::ERROR_UNEXPECTED_ELEMENT, "'<{}>'  is not a valid literal type.", element->Name());
        std::unreachable();
      },
      {{"f64", [](Parser* p, Element* e) -> pt::Literal { return p->f64(e); }},
       {"i8", [](Parser* p, Element* e) -> pt::Literal { return p->i8(e); }},
       {"i16", [](Parser* p, Element* e) -> pt::Literal { return p->i16(e); }},
       {"i32", [](Parser* p, Element* e) -> pt::Literal { return p->i32(e); }},
       {"i64", [](Parser* p, Element* e) -> pt::Literal { return p->i64(e); }},
       {"u8", [](Parser* p, Element* e) -> pt::Literal { return p->u8(e); }},
       {"u16", [](Parser* p, Element* e) -> pt::Literal { return p->u16(e); }},
       {"u32", [](Parser* p, Element* e) -> pt::Literal { return p->u32(e); }},
       {"u64", [](Parser* p, Element* e) -> pt::Literal { return p->u64(e); }}}};

    std::string_view name = element->Name();
    auto* literalParser = literalParsers.at(name);
    return literalParser(this, element);
  }
  pt::F64 Parser::f64(Element* element) {
    double value = 0.0;
    auto error = element->QueryDoubleText(&value);
    panicIf(error != tinyxml2::XML_SUCCESS,
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a numeric value in element '<f64>', found '{}'.",
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
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected an integer value in element, found '{}'.",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            diagnostic::Code::ERROR_NUMBER_OUT_OF_RANGE);
    diagnostic::emitInternalError("Control reached an impossible point");
  }
  pt::I16 Parser::i16(Element* element) {
    auto value = fe::parseNumber<pt::I16>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected an integer value in element, found {}",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            diagnostic::Code::ERROR_NUMBER_OUT_OF_RANGE);
    diagnostic::emitInternalError("Control reached an impossible point");
  }
  pt::I32 Parser::i32(Element* element) {
    auto value = fe::parseNumber<pt::I32>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected an integer value in element, found'{}'.",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            diagnostic::Code::ERROR_NUMBER_OUT_OF_RANGE);
    diagnostic::emitInternalError("Control reached an impossible point");
  }
  pt::I64 Parser::i64(Element* element) {
    auto value = fe::parseNumber<pt::I64>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected an integer value in element, found '{}'.",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            diagnostic::Code::ERROR_NUMBER_OUT_OF_RANGE);
    diagnostic::emitInternalError("Control reached an impossible point");
  }

  pt::U8 Parser::u8(Element* element) {
    auto value = fe::parseNumber<pt::U8>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected an unsigned integer value in element, found '{}'.",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            diagnostic::Code::ERROR_NUMBER_OUT_OF_RANGE);
    diagnostic::emitInternalError("Control reached an impossible point");
  }
  pt::U16 Parser::u16(Element* element) {
    auto value = fe::parseNumber<pt::U16>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected an unsigned integer value in element, found '{}'.",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            diagnostic::Code::ERROR_NUMBER_OUT_OF_RANGE);
    diagnostic::emitInternalError("Control reached an impossible point");
  }
  pt::U32 Parser::u32(Element* element) {
    auto value = fe::parseNumber<pt::U32>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected an unsigned integer value in element, found '{}'.",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            diagnostic::Code::ERROR_NUMBER_OUT_OF_RANGE);
    diagnostic::emitInternalError("Control reached an impossible point");
  }
  pt::U64 Parser::u64(Element* element) {
    auto value = fe::parseNumber<pt::U64>(element->GetText());
    if (value.has_value()) {
      return *value;
    }
    panicIf(value.error() == fe::NumberParseError::CANNOT_PARSE_NUMBER,
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected an unsigned integer value in element, found '{}'.",
            element->GetText());
    panicIf(value.error() == fe::NumberParseError::RESULT_OUT_OF_BOUNDS,
            element,
            diagnostic::Code::ERROR_NUMBER_OUT_OF_RANGE);
    diagnostic::emitInternalError("Control reached an impossible point");
  }

  std::string_view Parser::getAttribute(Element* element, std::string_view attribute) {
    auto value = element->Attribute(attribute.data());
    panicIf(value == nullptr,
            element,
            diagnostic::Code::ERROR_MISSING_ATTRIBUTE,
            "element <{}> is missing attribute '{}'.",
            element->Name(),
            attribute);

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

  ID Parser::parseId(Element* element) { return parseIdReference(element, "id"); }

  ID Parser::parseIdReference(Element* element, std::string_view attribute) {
    ID reference = INVALID_ID;
    auto error = element->QueryUnsigned64Attribute(attribute.data(), &reference);
    panicIf(error != tinyxml2::XML_SUCCESS,
            element,
            diagnostic::Code::ERROR_MISSING_ATTRIBUTE,
            "element <{}> is missing attribute '{}'.",
            element->Name(),
            attribute);

    return reference;
  }

  ID Parser::parseOptionalIdReference(Element* element, std::string_view attribute) {
    ID reference = INVALID_ID;
    element->QueryUnsigned64Attribute(attribute.data(), &reference);

    return reference;
  }

  FlowGraphLocation Parser::parseLocation(Element* element) {
    auto x = fe::parseNumber<int>(getAttribute(element, "x"));
    panicIf(!x.has_value(),
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a number in element location.x, found '{}'.",
            getAttribute(element, "x"));
    auto y = fe::parseNumber<int>(getAttribute(element, "y"));
    panicIf(!y.has_value(),
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a number in element location.y, found '{}'.",
            getAttribute(element, "y"));
    auto z = fe::parseNumber<int>(getAttribute(element, "z"));
    panicIf(!z.has_value(),
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a number in element location.z, found '{}'.",
            getAttribute(element, "z"));
    auto width = fe::parseNumber<int>(getAttribute(element, "w"));
    panicIf(!width.has_value(),
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a number in element location.width, found '{}'.",
            getAttribute(element, "w"));
    auto height = fe::parseNumber<int>(getAttribute(element, "h"));
    panicIf(!height.has_value(),
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a number in element location.height, found '{}'.",
            getAttribute(element, "h"));
    return {x.value(), y.value(), z.value(), width.value(), height.value()};
  }

  FlowGraphLocation Parser::parseBorderingLocation(Element* element) {
    auto xText = getOptionalAttribute(element, "x", "0");
    auto x = fe::parseNumber<int>(xText);
    panicIf(!x.has_value(),
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a number in element location.x, found '{}'.",
            xText);
    auto yText = getOptionalAttribute(element, "y", "0");
    auto y = fe::parseNumber<int>(yText);
    panicIf(!y.has_value(),
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a number in element location.y, found '{}'.",
            yText);

    auto width = fe::parseNumber<int>(getAttribute(element, "w"));
    panicIf(!width.has_value(),
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a number in element location.width, found '{}'.",
            getAttribute(element, "w"));
    auto height = fe::parseNumber<int>(getAttribute(element, "h"));
    panicIf(!height.has_value(),
            element,
            diagnostic::Code::ERROR_CANNOT_PARSE_ELEMENT_TEXT,
            "Expected a number in element location.height, found '{}'.",
            getAttribute(element, "h"));

    return FlowGraphLocation{
      .x = x.value(),
      .y = y.value(),
      .z = 0,
      .width = width.value(),
      .height = height.value(),
    };
  }

  Operator Parser::parseOperator(Element* element, std::string_view attribute) {
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
    } else if (opText == "++") {
      return Operator::PLUS_PLUS;
    } else if (opText == "--") {
      return Operator::MINUS_MINUS;
    } else {
      panicAt(element, diagnostic::Code::ERROR_UNRECOGNIZED_OPERATOR);
      std::unreachable();
    }
  }
}  // namespace fluir
