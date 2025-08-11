#ifndef FLUIR_COMPILER_DEBUG_ASG_PRINTER_HPP
#define FLUIR_COMPILER_DEBUG_ASG_PRINTER_HPP

#include <ostream>

#include "compiler/models/asg.hpp"
#include "compiler/utility/indent_formatter.hpp"

namespace fluir::debug {
  class AsgPrinter : private IndentFormatter<> {
   public:
    explicit AsgPrinter(std::ostream& out, bool inOrder = false);

    void print(const asg::ASG& asg);
    void print(const asg::DataFlowGraph& graph);

    void operator()(const asg::FunctionDecl& func);
    void operator()(const asg::BinaryOp& binary);
    void operator()(const asg::UnaryOp& unary);
    void operator()(const asg::ConstantFP& constant);

   private:
    std::ostream& out_;
    bool inOrder_;

    void doOutOfOrderPrint(const asg::DataFlowGraph& graph);
    void doInOrderPrint(const asg::DataFlowGraph& graph);
  };
}  // namespace fluir::debug

#endif
