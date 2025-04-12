#include "fluir/compiler/parser.hpp"

#include <fstream>
#include <sstream>

#include "fluir/compiler/diagnostic.hpp"

namespace fluir::compiler {
  template <typename... Args>
  void Parser::panicIf(bool condition, Args&&... errorMessage) {
    if (condition) {
      emitError(diagnostics_,
                std::forward<Args>(errorMessage)...);
      throw BadParse{};
    }
  }

  template <typename... Args>
  void Parser::panic(Args&&... errorMessage) {
    emitError(diagnostics_,
              std::forward<Args>(errorMessage)...);
    throw BadParse{};
  }

  fluir::ast::AST Parser::parseFile(const std::filesystem::path& program) {
    reset(program.filename().c_str());
    std::ifstream fin{program};
    std::stringstream ss;
    ss << fin.rdbuf();
    std::string source = ss.str();

    source_.Parse(source.c_str());
    root();

    return std::move(ast_);
  }

  fluir::ast::AST Parser::parse(const std::string_view src) {
    reset("<UNKNOWN>");
    source_.Parse(src.data());

    root();

    return std::move(ast_);
  }

  void Parser::reset(const std::string_view filename) {
    filename_ = filename;
    source_.Clear();
    diagnostics_.clear();
    ast_ = ast::AST{};
  }

  void Parser::root() {
    static std::string expectedRoot = "fluir";
    auto root = source_.RootElement();
    if (root->Name() != expectedRoot) {
      emitError(diagnostics_,
                std::make_unique<SourceLocation>(root->GetLineNum(), filename_),
                "Expected root element to be '{}', found '{}'.",
                expectedRoot, root->Name());
      // This document is likely not a fluir source file. So just bail out
      return;
    }

    for (auto child = root->FirstChildElement();
         child != NULL;
         child = child->NextSiblingElement()) {
      declaration(child);
    }
  }

  void Parser::declaration(Element* element) {
    const std::string FUNCTION{"fl:function"};
    try {
      if (element->Name() == FUNCTION) {
        function(element);
      } else {
        emitError(diagnostics_,
                  std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                  "Unexpected element '{}'. Expected declaration.", element->Name());
      }
    } catch (const BadParse&) {
      // Ignore the rest of this element to try to prevent cascading errors
    }
  }

  void Parser::function(Element* element) {
    constexpr std::string_view type = "fl:function";
    std::string_view name = getAttribute(element, type, "name");
    fluir::id_t id = std::atoll(getAttribute(element, type, "id").data());
    auto location = parseLocation(element, type);

    auto bodyElement = element->FirstChildElement("body");
    ast::Block body;
    if (!bodyElement) {
      emitError(diagnostics_,
                std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                "Function 'foo' has no body. Expected a '<body>' element.", name);
    } else {
      body = block(bodyElement->FirstChildElement());
    }

    if (ast_.declarations.contains(id)) {
      emitError(diagnostics_,
                std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                "Duplicate declaration IDs. Function '{}' has id {}, but that ID is already in use.",
                name, id);
    }
    ast_.declarations.emplace(id, ast::FunctionDecl{std::string(name), id, body, location});
  }

  ast::Block Parser::block(Element* element) {
    auto block = ast::EMPTY_BLOCK;
    for (; element != nullptr; element = element->NextSiblingElement()) {
      std::string_view name = element->Name();
      if (name == "fl:constant") {
        auto node = constant(element);
        panicIf(block.nodes.contains(node.id),
                std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                "Duplicate node ids. Node <{}> has id {}, but that ID is already in use.",
                name, node.id);
        block.nodes.emplace(node.id, std::move(node));
      } else if (name == "fl:binary") {
        auto node = binary(element);
        panicIf(block.nodes.contains(node.id),
                std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                "Duplicate node ids. Node <{}> has id {}, but that ID is already in use.",
                name, node.id);
        block.nodes.emplace(node.id, std::move(node));
      } else {
        panicIf(true,
                std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                "Unexpected element '{}'. Expected a node.", name);
      }
    }
    return block;
  }

  ast::Binary Parser::binary(Element* element) {
    constexpr std::string_view type = "fl:binary";
    fluir::id_t id = std::atoll(getAttribute(element, type, "id").data());
    auto location = parseLocation(element, type);
    fluir::id_t lhsId = std::atoll(getAttribute(element, type, "lhs").data());
    fluir::id_t rhsId = std::atoll(getAttribute(element, type, "rhs").data());
    ast::Operator op = ast::Operator::UNKNOWN;
    if (auto opStr = getAttribute(element, type, "operator");
        opStr == "+") {
      op = ast::Operator::PLUS;
    } else if (opStr == "-") {
      op = ast::Operator::MINUS;
    } else if (opStr == "*") {
      op = ast::Operator::STAR;
    } else if (opStr == "/") {
      op = ast::Operator::SLASH;
    } else {
      panic(std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
            "Unrecognized operator '{}' in node <{}>.", opStr, type);
    }

    return ast::Binary{.id = id, .lhs = lhsId, .rhs = rhsId, .op = op, .location = location};
  }

  ast::Constant Parser::constant(Element* element) {
    constexpr std::string_view type = "fl:constant";
    fluir::id_t id = std::atoll(getAttribute(element, type, "id").data());
    auto location = parseLocation(element, type);
    auto val = value(element->FirstChildElement());

    return ast::Constant{.value = val, .id = id, .location = location};
  }

  ast::Value Parser::value(Element* element) {
    std::string_view name = element->Name();
    if (name == "fl:double") {
      double val = 0.0;
      auto error = element->QueryDoubleText(&val);
      if (error) {
        emitError(diagnostics_,
                  std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                  "Expected a numeric value in element '<fl:double>'. '{}' cannot be parsed as a number.",
                  element->GetText());
      }
      return val;
    }
    // TODO: Handle other types. Should this be an error, or parse as an identifier?
    panicIf(true, std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
            "TODO: Handle this more gracefully. Unknown value type '{}'",
            name);
    return {};
  }

  fluir::id_t Parser::parseId(Element* element, std::string_view type) {
    auto id = element->Unsigned64Attribute("id", fluir::INVALID_ID);
    panicIf(id == INVALID_ID,
            std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
            "{} element is missing attribute 'id'.",
            type);
    return id;
  }

  ast::LocationInfo Parser::parseLocation(Element* element, std::string_view type) {
    return {
        .x = std::atoi(getAttribute(element, type, "x").data()),
        .y = std::atoi(getAttribute(element, type, "y").data()),
        .z = std::atoi(getAttribute(element, type, "z").data()),
        .width = std::atoi(getAttribute(element, type, "h").data()),
        .height = std::atoi(getAttribute(element, type, "w").data()),
    };
  }

  std::string_view Parser::getAttribute(Element* element, std::string_view type, std::string_view attribute) {
    auto value = element->Attribute(attribute.data());
    panicIf(value == nullptr,
            std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
            "{} element is missing attribute '{}'.",
            type, attribute);

    return value;
  }

  std::string Parser::SourceLocation::string() const {
    return fmt::format("on line {} of '{}'", lineNo, filename);
  }
}  // namespace fluir::compiler
