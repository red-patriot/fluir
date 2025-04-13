#include "fluir/compiler/parser.hpp"

#include <fstream>
#include <sstream>

#include "fluir/compiler/diagnostic.hpp"

namespace fluir::compiler {
  template <typename... Args>
  void Parser::panicIf(bool condition, Element* element, Args&&... errorMessage) {
    if (condition) {
      emitError(diagnostics_,
                std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                std::forward<Args>(errorMessage)...);
      throw Panic{};
    }
  }

  template <typename... Args>
  void Parser::panic(Element* element, Args&&... errorMessage) {
    emitError(diagnostics_,
              std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
              std::forward<Args>(errorMessage)...);
    throw Panic{};
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

  fluir::ast::AST Parser::parseString(const std::string_view src) {
    reset("<FROM STRING>");
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
        panic(element,
              "Unexpected element '{}'. Expected declaration.", element->Name());
      }
    } catch (const Panic&) {
      // Ignore the rest of this element to try to prevent cascading errors
    }
  }

  void Parser::function(Element* element) {
    constexpr std::string_view type = "fl:function";
    std::string_view name = getAttribute(element, type, "name");
    fluir::id_t id = std::atoll(getAttribute(element, type, "id").data());
    auto location = parseLocation(element, type);

    auto bodyElement = element->FirstChildElement("body");
    panicIf(!bodyElement, element,
            "Function '{}' has no body. Expected a '<body>' element.", name);

    ast::Block body = block(bodyElement->FirstChildElement());

    panicIf(ast_.declarations.contains(id),
            element,
            "Duplicate declaration IDs. Function '{}' has id {}, but that ID is already in use.",
            name, id);
    ast_.declarations.emplace(id, ast::FunctionDecl{std::string(name), id, body, location});
  }

  ast::Block Parser::block(Element* element) {
    auto block = ast::EMPTY_BLOCK;
    for (; element != nullptr; element = element->NextSiblingElement()) {
      try {
        std::string_view name = element->Name();
        if (name == "fl:constant") {
          auto node = constant(element);
          panicIf(block.nodes.contains(node.id),
                  element,
                  "Duplicate node ids. Node <{}> has id {}, but that ID is already in use.",
                  name, node.id);
          block.nodes.emplace(node.id, std::move(node));
        } else if (name == "fl:unary") {
          auto node = unary(element);
          panicIf(block.nodes.contains(node.id),
                  element,
                  "Duplicate node ids. Node <{}> has id {}, but that ID is already in use.",
                  name, node.id);
          block.nodes.emplace(node.id, std::move(node));
        } else if (name == "fl:binary") {
          auto node = binary(element);
          panicIf(block.nodes.contains(node.id),
                  element,
                  "Duplicate node ids. Node <{}> has id {}, but that ID is already in use.",
                  name, node.id);
          block.nodes.emplace(node.id, std::move(node));
        } else {
          panicIf(true,
                  element,
                  "Unexpected element '{}'. Expected a node.", name);
        }
      } catch (const Panic&) {
        // Synchronize after each node
      }
    }
    return block;
  }

  ast::Unary Parser::unary(Element* element) {
    constexpr std::string_view type = "fl:unary";
    fluir::id_t id = std::atoll(getAttribute(element, type, "id").data());
    auto location = parseLocation(element, type);
    fluir::id_t lhsId = std::atoll(getAttribute(element, type, "lhs").data());
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
      panic(element,
            "Unrecognized operator '{}' in node <{}>.", opStr, type);
    }

    return ast::Unary{.id = id, .lhs = lhsId, .op = op, .location = location};
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
      panic(element,
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
        panic(element,
              "Expected a numeric value in element '<fl:double>'. '{}' cannot be parsed as a number.",
              element->GetText());
      }
      return val;
    }
    // TODO: Handle other types. Should this be an error, or parse as an identifier?
    panic(element,
          "TODO: Handle this more gracefully. Unknown value type '{}'",
          name);
    return {};
  }

  fluir::id_t Parser::parseId(Element* element, std::string_view type) {
    auto id = element->Unsigned64Attribute("id", fluir::INVALID_ID);
    panicIf(id == INVALID_ID,
            element,
            "{} element is missing attribute 'id'.",
            type);
    return id;
  }

  ast::LocationInfo Parser::parseLocation(Element* element, std::string_view type) {
    return {
        .x = std::atoi(getAttribute(element, type, "x").data()),
        .y = std::atoi(getAttribute(element, type, "y").data()),
        .z = std::atoi(getAttribute(element, type, "z").data()),
        .width = std::atoi(getAttribute(element, type, "w").data()),
        .height = std::atoi(getAttribute(element, type, "h").data()),
    };
  }

  std::string_view Parser::getAttribute(Element* element, std::string_view type, std::string_view attribute) {
    auto value = element->Attribute(attribute.data());
    panicIf(value == nullptr,
            element,
            "{} element is missing attribute '{}'.",
            type, attribute);

    return value;
  }

  std::string Parser::SourceLocation::string() const {
    return fmt::format("on line {} of '{}'", lineNo, filename);
  }
}  // namespace fluir::compiler
