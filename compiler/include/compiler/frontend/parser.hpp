#ifndef FLUIR_COMPILER_FRONTEND_PARSER_HPP
#define FLUIR_COMPILER_FRONTEND_PARSER_HPP

#include <filesystem>
#include <string>
#include <utility>

#include <tinyxml2.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/context.hpp"
#include "compiler/utility/results.hpp"

namespace fluir {
  Results<pt::ParseTree> parseString(Context& ctx, const std::string_view source);
  Results<pt::ParseTree> parseFile(Context& ctx, const std::filesystem::path& source);

  class Parser {
   public:
    explicit Parser(Context& ctx);

    Results<pt::ParseTree> parseString(const std::string_view source);
    Results<pt::ParseTree> parseFile(const std::filesystem::path& file);

   private:
    Context& ctx_;
    tinyxml2::XMLDocument doc_;
    pt::ParseTree tree_;
    bool failed{false};

    using Element = tinyxml2::XMLElement;

    bool flowGraph();

    void header(Element* element);
    Version version(Element* element);

    void comment(Element* element);

    void declaration(Element* element);
    void functionDecl(Element* element);

    pt::FunctionDecl::InputBlock funcInputs(Element* element);
    pt::FunctionDecl::Parameter funcParameter(Element* element, int index);
    pt::FunctionDecl::OutputBlock funcOutputs(Element* element);
    pt::FunctionDecl::Return funcReturn(Element* element);

    pt::Block block(Element* element);
    std::optional<WithID<pt::Node>> node(Element* element);
    WithID<pt::Node> constant(Element* element);
    WithID<pt::Node> binary(Element* element);
    WithID<pt::Node> unary(Element* element);
    WithID<pt::Node> call(Element* element);

    WithID<pt::Conduit> conduit(Element* element);
    pt::Conduit::Output conduitOutput(Element* element);

    pt::Literal literal(Element* element);
    pt::F64 f64(Element* element);
    pt::I8 i8(Element* element);
    pt::I16 i16(Element* element);
    pt::I32 i32(Element* element);
    pt::I64 i64(Element* element);
    pt::U8 u8(Element* element);
    pt::U16 u16(Element* element);
    pt::U32 u32(Element* element);
    pt::U64 u64(Element* element);

    std::string_view getAttribute(Element* element, std::string_view attribute);
    std::string_view getOptionalAttribute(Element* element,
                                          std::string_view attribute,
                                          std::string_view defaultValue = "");
    ID parseId(Element* element);
    ID parseIdReference(Element* element, std::string_view attribute);
    ID parseOptionalIdReference(Element* element, std::string_view attribute);
    FlowGraphLocation parseLocation(Element* element);
    Operator parseOperator(Element* element, std::string_view attribute);

    template <typename... FmtArgs>
    void panicAt(Element*, diagnostic::Code code, fmt::format_string<FmtArgs...> format = "", FmtArgs&&... args);
    template <typename... FmtArgs>
    void panicIf(
      bool condition, Element*, diagnostic::Code code, fmt::format_string<FmtArgs...> format = "", FmtArgs&&... args);
  };
}  // namespace fluir

#endif
