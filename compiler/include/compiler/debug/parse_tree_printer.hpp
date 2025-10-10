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
    void operator()(const pt::Binary& binary);
    void operator()(const pt::Unary& unary);
    void operator()(const pt::Constant& constant);
    void operator()(const pt::Conduit& conduit);

   private:
    std::ostream& out_;

    std::string doPrint(const FlowGraphLocation& loc);
  };
}  // namespace fluir::debug

#endif  // FLUIR_PARSE_TREE_PRINTER_HPP
