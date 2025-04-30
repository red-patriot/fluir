#include "compiler/backend/bytecode_generator.hpp"

using Inst = fluir::code::Instruction;

namespace fluir {
  Results<code::ByteCode> generateCode(const asg::ASG& graph) {
    return BytecodeGenerator::generate(graph);
  }

  Results<code::ByteCode> BytecodeGenerator::generate(const asg::ASG& graph) {
    BytecodeGenerator generator{graph};
    return generator.run();
  }

  void BytecodeGenerator::operator()(const asg::FunctionDecl& func) {
    code::Chunk generated;
    generated.name = func.name;

    // TODO: Generate for each statement

    // (FOR NOW) end all functions with the EXIT instruction
    generated.code.push_back(Inst::EXIT);
    code_.chunks.push_back(std::move(generated));
  }

  void BytecodeGenerator::operator()(const asg::BinaryOp&) { }

  void BytecodeGenerator::operator()(const asg::UnaryOp&) { }

  void BytecodeGenerator::operator()(const asg::ConstantFP&) { }

  Results<code::ByteCode> BytecodeGenerator::run() {
    for (const auto& declaration : graph_.declarations) {
      (*this)(declaration);
    }

    // For now, zero out header
    code_.header = code::Header{};

    return {std::move(code_), std::move(diagnostics_)};
  }

  BytecodeGenerator::BytecodeGenerator(const asg::ASG& graph) :
      graph_(graph),
      code_{} { }
}  // namespace fluir
