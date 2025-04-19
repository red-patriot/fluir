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
  void Parser::panicAt(Element* element,
                       std::string_view format,
                       FmtArgs&&... args) {
    diagnostics_.emitError(
        fmt::vformat(format, fmt::make_format_args(std::forward<FmtArgs>(args)...)),
        std::make_shared<SourceLocation>(element->GetLineNum(), filename_));
    throw PanicMode{};
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

    return {std::move(tree_), std::move(diagnostics_)};
  }

  Results<pt::ParseTree> Parser::parseString(const std::string_view source) {
    filename_ = "<FROM STRING>";
    doc_.Clear();
    doc_.Parse(source.data());

    flowGraph();

    return {std::move(tree_), std::move(diagnostics_)};
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
        pt::FunctionDecl{location, std::string(name), body});
  }

  pt::Block Parser::block(Element*) {
    auto block = pt::EMPTY_BLOCK;
    // TODO: Parse elements here
    return block;
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
    auto id = element->Unsigned64Attribute("id", fluir::INVALID_ID);
    return id;
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

  std::string Parser::SourceLocation::str() const {
    return fmt::format("on line {} of '{}'", lineNo, filename);
  }
}  // namespace fluir
