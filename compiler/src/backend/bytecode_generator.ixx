module;

#include "bytecode/byte_code.hpp"
#include "compiler/utility/results.hpp"

export module fluir.backend.bytecode_generator;
export import fluir.backend.code_writer;
export import fluir.models.asg;

namespace fluir {
  export Results<code::ByteCode> generateCode(const asg::ASG& graph);
  export void writeCode(const code::ByteCode& code, CodeWriter& writer, std::ostream& destination);

  export class BytecodeGenerator {
   public:
    static Results<code::ByteCode> generate(const asg::ASG& graph);

    void operator()(const asg::FunctionDecl& func);
    void operator()(const asg::BinaryOp& binary);
    void operator()(const asg::UnaryOp& unary);
    void operator()(const asg::ConstantFP& constant);

   private:
    const asg::ASG& graph_;
    code::ByteCode code_;
    Diagnostics diagnostics_;
    code::Chunk current_;

    explicit BytecodeGenerator(const asg::ASG& graph);

    void emitByte(std::uint8_t byte);
    void emitBytes(std::uint8_t byte1, std::uint8_t byte2);
    size_t addConstant(code::Value value);

    Results<code::ByteCode> run();
    void doTopLevel(const asg::Node& node);
  };
}  // namespace fluir
