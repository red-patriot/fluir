#ifndef FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP
#define FLUIR_COMPILER_BACKEND_BYTECODE_GENERATOR_HPP

#include "bytecode/byte_code.hpp"
#include "compiler/backend/code_writer.hpp"
#include "compiler/models/asg.hpp"
#include "compiler/utility/results.hpp"

namespace fluir {
  Results<code::ByteCode> generateCode(const asg::ASG& graph);

  class BytecodeGenerator {
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

    Results<code::ByteCode> run();

    explicit BytecodeGenerator(const asg::ASG& graph);
  };
}  // namespace fluir

#endif
