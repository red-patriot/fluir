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
                "Expect root element to be '{}}', found '{}'.",
                expectedRoot, root->Name());
    }

    for (auto child = root->FirstChildElement();
         child != NULL;
         child = child->NextSiblingElement()) {
      declaration(child);
    }
  }

  void Parser::declaration(Element* element) {
    const std::string FUNCTION{"fl:function"};
    if (element->Name() == FUNCTION) {
      function(element);
    } else {
      emitError(diagnostics_,
                std::make_unique<SourceLocation>(element->GetLineNum(), filename_),
                "Unexpected element '{}'. Expected declaration.", element->Name());
    }
  }

  void Parser::function(Element* element) {
    // TODO: Check for errors
    std::string_view name = element->Attribute("name");
    fluir::id_t id = std::atoll(element->Attribute("id"));
    auto location = parseLocation(element);

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
    fluir::id_t id = std::atoll(element->Attribute("id"));
    auto location = parseLocation(element);
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

  ast::LocationInfo Parser::parseLocation(Element* element) {
    // TODO: Check for errors
    return {
        .x = std::atoi(element->Attribute("x")),
        .y = std::atoi(element->Attribute("y")),
        .z = std::atoi(element->Attribute("z")),
        .width = std::atoi(element->Attribute("h")),
        .height = std::atoi(element->Attribute("w")),
    };
  }

  std::string Parser::SourceLocation::string() const {
    return fmt::format("on line {} of '{}'", lineNo, filename);
  }
}  // namespace fluir::compiler
