#include "fluir/compiler/parser.hpp"

#include <fstream>
#include <sstream>

#include "fluir/compiler/diagnostic.hpp"

namespace fluir::compiler {
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
    // TODO: Check for errors
    constexpr std::string_view type = "fl:function";
    std::string_view name = getAttribute(element, type, "name");
    fluir::id_t id = std::atoll(getAttribute(element, type, "id").data());
    auto location = parseLocation(element, type);

    // TODO: Parse body
    auto bodyElement = element->FirstChildElement("body");
    if (!bodyElement) {
      // TODO: Emit error
    }
    auto body = block(bodyElement->FirstChildElement());
    // TODO: Check for duplicates
    ast_.declarations.emplace(id, ast::FunctionDecl{std::string(name), id, body, location});
  }

  ast::Block Parser::block([[maybe_unused]] Element* element) {
    auto block = ast::EMPTY_BLOCK;
    for (; element != nullptr; element = element->NextSiblingElement()) {
      // TODO: Parse multiple nodes correctly
      std::string_view name = element->Name();
      if (name == "fl:constant") {
        block.node = constant(element);
      }
    }
    return block;
  }

  ast::Constant Parser::constant(Element* element) {
    // TODO: Check for errors
    fluir::id_t id = std::atoll(getAttribute(element, "fl:constant", "id").data());
    auto location = parseLocation(element, "fl:constant");
    auto val = value(element->FirstChildElement());

    return ast::Constant{.value = val, .id = id, .location = location};
  }

  ast::Value Parser::value(Element* element) {
    std::string_view name = element->Name();
    if (name == "fl:double") {
      double val;
      auto error = element->QueryDoubleText(&val);
      // TODO: check error
      return val;
    }
    // TODO: ERROR on unknown
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
    if (value == nullptr) {
      emitError(diagnostics_,
                std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                "{} element is missing attribute '{}'.",
                type, attribute);
      throw BadParse{};
    }
    return value;
  }

  std::string Parser::SourceLocation::string() const {
    return fmt::format("on line {} of '{}'", lineNo, filename);
  }
}  // namespace fluir::compiler
