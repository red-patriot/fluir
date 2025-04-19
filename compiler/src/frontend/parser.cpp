#include "compiler/frontend/parser.hpp"

namespace fluir {
  Results<pt::ParseTree> parseString(const std::string_view source) {
    Parser parser{};
    return parser.parseString(source);
  }

  Results<pt::ParseTree> Parser::parseString(const std::string_view source) {
    doc_.Clear();
    doc_.Parse(source.data());

    flowGraph();

    return {std::move(tree_), std::move(diagnostics_)};
  }

  void Parser::flowGraph() {
    auto root = doc_.RootElement();

    // TODO: Check root name
    // TODO: Check metadata

    for (auto child = root->FirstChildElement();
         child != nullptr;
         child = child->NextSiblingElement()) {
      declaration(child);
    }
  }

  void Parser::declaration(Element* element) {
    std::string_view name = element->Name();
    if (name == "fl:function") {
      functionDecl(element);
    } else {
      // TODO: ERROR
    }
  }

  void Parser::functionDecl(Element* element) {
    constexpr std::string_view type = "fl:function";
    std::string_view name = getAttribute(element, type, "name");
    ID id = parseId(element, type);
    auto location = parseLocation(element, type);

    auto bodyElement = element->FirstChildElement("body");
    // panicIf(!bodyElement, element,
    //         "Function '{}' has no body. Expected a '<body>' element.", name);

    pt::Block body = block(bodyElement->FirstChildElement());

    // panicIf(tree_.declarations.contains(id),
    //         element,
    //         "Duplicate declaration IDs. Function '{}' has id {}, but that ID is already in use.",
    //         name, id);
    tree_.declarations.emplace(
        id,
        pt::FunctionDecl{location, std::string(name), body});
  }

  pt::Block Parser::block(Element*) {
    auto block = pt::EMPTY_BLOCK;
    // TODO: Parse elements here
    return block;
  }

  std::string_view Parser::getAttribute(Element* element,
                                        std::string_view type,
                                        std::string_view attribute) const {
    auto value = element->Attribute(attribute.data());
    // panicIf(value == nullptr,
    //         element,
    //         "{} element is missing attribute '{}'.",
    //         type, attribute);

    return value;
  }

  ID Parser::parseId(Element* element, std::string_view type) const {
    auto id = element->Unsigned64Attribute("id", fluir::INVALID_ID);
    return id;
  }

  FlowGraphLocation Parser::parseLocation(Element* element, std::string_view type) const {
    return {
        .x = std::atoi(getAttribute(element, type, "x").data()),
        .y = std::atoi(getAttribute(element, type, "y").data()),
        .z = std::atoi(getAttribute(element, type, "z").data()),
        .width = std::atoi(getAttribute(element, type, "w").data()),
        .height = std::atoi(getAttribute(element, type, "h").data()),
    };
  }
}  // namespace fluir
