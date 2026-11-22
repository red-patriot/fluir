#ifndef FLUIR_COMPILER_DEBUG_PARSE_TREE_PRINTER_HPP
#define FLUIR_COMPILER_DEBUG_PARSE_TREE_PRINTER_HPP

#include <ostream>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/utility/indent_formatter.hpp"

namespace fluir::debug {
  class ParseTreePrinter : private IndentFormatter<4> {
   public:
    explicit ParseTreePrinter(std::ostream& out);

    void print(const pt::ParseTree& tree);

    void operator()(const pt::FunctionDecl& func);
    void operator()(const pt::FunctionDecl::Parameter& param);
    void operator()(const pt::FunctionDecl::Return& ret);
    void operator()(const pt::FunctionDecl::InputBlock& input);
    void operator()(const pt::FunctionDecl::OutputBlock& output);

    void operator()(const pt::Binary& binary);
    void operator()(const pt::Unary& unary);
    void operator()(const pt::Constant& constant);
    void operator()(const pt::Call& call);
    void operator()(const pt::Conditional& conditional);

    void operator()(const pt::Conduit& conduit);

    void operator()(const pt::Comment& comment);

   private:
    std::ostream& out_;

    std::string doPrint(const FlowGraphLocation& loc);
    void printBlock(const pt::Block& block);
  };
}  // namespace fluir::debug

template <>
struct fmt::formatter<fluir::pt::Constant> {
  template <class ParseContext>
  constexpr ParseContext::iterator parse(ParseContext& ctx) {
    return ctx.begin();
  }

  template <class FmtContext>
  FmtContext::iterator format(fluir::pt::Constant c, FmtContext& ctx) const {
    fmt::format_to(ctx.out(), "{} ", fluir::pt::toString(c.typeTag));
    switch (c.typeTag) {
      default:
        return fmt::format_to(ctx.out(), "{}", c.value);
      case fluir::pt::Constant::Type::STR:
        return fmt::format_to(ctx.out(), "\"{}\"", c.value);
    }
  }
};

#endif  // FLUIR_COMPILER_DEBUG_PARSE_TREE_PRINTER_HPP
