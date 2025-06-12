module;
#include <ostream>

export module fluir.debug.asg_printer;
export import fluir.models.asg;
import fluir.utility.indent_formatter;

namespace fluir::debug {
  export class AsgPrinter : private IndentFormatter<> {
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
