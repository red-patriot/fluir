#ifndef FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP
#define FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP

#include "bytecode/byte_code.hpp"
#include "compiler/backend/code_writer.hpp"
#include "compiler/models/asg.hpp"
#include "compiler/utility/context.hpp"

namespace fluir {
  Results<code::ByteCode> generateCode(Context& ctx, const asg::ASG& graph);
  void writeCode(const code::ByteCode& code, CodeWriter& writer, std::ostream& destination);

  class BytecodeGenerator {
   public:
    static Results<code::ByteCode> generate(Context& ctx, const asg::ASG& graph);

    void operator()(const asg::FunctionDecl& func);
    void operator()(const asg::BinaryOp& binary);
    void operator()(const asg::UnaryOp& unary);
    void operator()(const asg::ConstantFP& constant);

   private:
    Context& ctx_;
    const asg::ASG& graph_;
    code::ByteCode code_;
    code::Chunk current_;

    explicit BytecodeGenerator(Context& ctx, const asg::ASG& graph);

    void emitByte(std::uint8_t byte);
    void emitBytes(std::uint8_t byte1, std::uint8_t byte2);
    size_t addConstant(code::Value value);

    Results<code::ByteCode> run();
    void doTopLevel(const asg::Node& node);
  };
}  // namespace fluir

#endif
