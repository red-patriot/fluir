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

    void operator()(const pt::F64& f64);
    void operator()(const pt::I8& i8);
    void operator()(const pt::I16& i16);
    void operator()(const pt::I32& i32);
    void operator()(const pt::I64& i64);
    void operator()(const pt::U8& u8);
    void operator()(const pt::U16& u16);
    void operator()(const pt::U32& u32);
    void operator()(const pt::U64& u64);

   private:
    std::ostream& out_;

    std::string doPrint(const FlowGraphLocation& loc);
  };
}  // namespace fluir::debug

#endif  // FLUIR_COMPILER_DEBUG_PARSE_TREE_PRINTER_HPP
