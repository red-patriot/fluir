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
    std::string_view name = element->Attribute("name");
    fluir::id_t id = std::atoll(element->Attribute("id"));

    // TODO: Parse location
    // TODO: Parse body

    ast_.declarations.emplace_back(ast::makeFunctionDecl(name, id));
  }
}  // namespace fluir::compiler
