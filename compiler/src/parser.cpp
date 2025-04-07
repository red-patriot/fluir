#include "fluir/compiler/parser.hpp"

#include "fluir/compiler/diagnostic.hpp"

namespace fluir::compiler {
  fluir::ast::AST Parser::parse(const std::string_view src) {
    ast_ = ast::AST{};
    source_.Parse(src.data());

    root();

    return std::move(ast_);
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
                "Unexpected element '{}'. Expect declaration.", element->Name());
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
    // TODO: Parse a block
    return ast::EMPTY_BLOCK;
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
}  // namespace fluir::compiler
