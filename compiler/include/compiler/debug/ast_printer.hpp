#ifndef FLUIR_COMPILER_DEBUG_AST_PRINTER_HPP
#define FLUIR_COMPILER_DEBUG_AST_PRINTER_HPP

#include <ostream>

#include "compiler/models/ast.hpp"
#include "compiler/utility/indent_formatter.hpp"

namespace fluir::debug {
  class AstPrinter : private IndentFormatter<> {
   public:
    explicit AstPrinter(std::ostream& out, bool inOrder = false);

    void print(const ast::AST& ast);
    void print(const ast::DataFlowGraph& graph);

    void operator()(const ast::FunctionDecl& func);
    void operator()(const ast::BinaryOp& binary);
    void operator()(const ast::UnaryOp& unary);
    void operator()(const ast::Constant& constant);
    void operator()(const ast::Cast& cast);
    void operator()(const ast::LocalWrite& write);
    void operator()(const ast::LocalRead& read);
    void operator()(const ast::Call& call);

   private:
    std::ostream& out_;
    bool inOrder_;

    void doOutOfOrderPrint(const ast::DataFlowGraph& graph);
    void doInOrderPrint(const ast::DataFlowGraph& graph);

    void print(const ast::Node& node);
  };
}  // namespace fluir::debug

#endif
