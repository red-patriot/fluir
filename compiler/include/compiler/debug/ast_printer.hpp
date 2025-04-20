#ifndef FLUIR_COMPILER_DEBUG_AST_PRINTER_HPP
#define FLUIR_COMPILER_DEBUG_AST_PRINTER_HPP

#include <ostream>

#include "compiler/frontend/ast/ast.hpp"

namespace fluir::debug {
  class AstPrinter {
   public:
    explicit AstPrinter(std::ostream& out, bool inOrder = false);

    void print(const ast::ASG& asg);
    void print(const ast::DataFlowGraph& graph);

    void operator()(const ast::FunctionDecl& func);
    void operator()(const ast::BinaryOp& binary);
    void operator()(const ast::UnaryOp& unary);
    void operator()(const ast::ConstantFP& constant);

   private:
    std::ostream& out_;
    bool inOrder_;
    int indent_{0};

    void doOutOfOrderPrint(const ast::DataFlowGraph& graph);
    void doInOrderPrint(const ast::DataFlowGraph& graph);

    std::string indent();
  };
}  // namespace fluir::debug

#endif
