#include "compiler/backend/bytecode_generator.hpp"

#include <cstdint>

#include <fmt/format.h>

using fluir::code::Instruction;

namespace fluir {
  Results<code::ByteCode> generateCode(Context& ctx, const asg::ASG& graph) {
    return BytecodeGenerator::generate(ctx, graph);
  }

  void writeCode(const code::ByteCode& code, CodeWriter& writer, std::ostream& destination) {
    return writer.write(code, destination);
  }

  Results<code::ByteCode> BytecodeGenerator::generate(Context& ctx, const asg::ASG& graph) {
    BytecodeGenerator generator{ctx, graph};
    return generator.run();
  }

  void BytecodeGenerator::operator()(const asg::FunctionDecl& func) {
    current_ = code::Chunk{};
    current_.name = func.name;

    for (const auto& node : func.statements) {
      doTopLevel(node);
    }

    // (FOR NOW) end all functions with the EXIT instruction
    emitByte(Instruction::EXIT);
    code_.chunks.push_back(std::move(current_));
  }

  void BytecodeGenerator::operator()(const asg::BinaryOp& node) {
    std::visit(*this, *node.lhs);
    std::visit(*this, *node.rhs);

    // TODO: Handle other types here
    switch (node.op) {
      case Operator::PLUS:
        emitByte(Instruction::F64_ADD);
        break;
      case Operator::MINUS:
        emitByte(Instruction::F64_SUB);
        break;
      case Operator::STAR:
        emitByte(Instruction::F64_MUL);
        break;
      case Operator::SLASH:
        emitByte(Instruction::F64_DIV);
        break;
      case Operator::UNKNOWN:
        // TODO: Handle this better
        ctx_.diagnostics.emitError("Unknown operator encountered. Expected one of +, -, *, /");
        break;
    }
  }

  void BytecodeGenerator::operator()(const asg::UnaryOp& node) {
    std::visit(*this, *node.operand);
    // TODO: Handle other types here
    switch (node.op) {
      case Operator::PLUS:
        emitByte(Instruction::F64_AFF);
        break;
      case Operator::MINUS:
        emitByte(Instruction::F64_NEG);
        break;
      default:
        // TODO: Handle this better
        ctx_.diagnostics.emitError("Unknown operator encountered. Expected one of +, -");
        break;
    }
  }

  void BytecodeGenerator::operator()(const asg::ConstantFP& node) {
    const auto constant = addConstant(node.value);

    // TODO: Handle more constants with special instruction
    emitBytes(Instruction::PUSH, static_cast<std::uint8_t>(constant));
  }

  BytecodeGenerator::BytecodeGenerator(Context& ctx, const asg::ASG& graph) : ctx_(ctx), graph_(graph), code_{} { }

  void BytecodeGenerator::emitByte(std::uint8_t byte) { current_.code.push_back(byte); }
  void BytecodeGenerator::emitBytes(std::uint8_t byte1, std::uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
  }
  size_t BytecodeGenerator::addConstant(code::Value value) {
    if (auto found = std::ranges::find(current_.constants, value); found != current_.constants.end()) {
      return found - current_.constants.begin();
    }
    current_.constants.emplace_back(std::move(value));
    if (current_.constants.size() > UINT8_MAX) {
      ctx_.diagnostics.emitError(fmt::format("Too many constants. Only {} constants allowed.", UINT8_MAX));
    }
    return current_.constants.size() - 1;
  }

  Results<code::ByteCode> BytecodeGenerator::run() {
    for (const auto& declaration : graph_.declarations) {
      (*this)(declaration);
    }

    // For now, zero out header
    // TODO: Put the language version in the header here
    code_.header = code::Header{};

    return std::move(code_);
  }

  void BytecodeGenerator::doTopLevel(const asg::Node& node) {
    std::visit(*this, node);

    // Each top level node will leave a value on the stack, so pop it off
    emitByte(Instruction::POP);
  }
}  // namespace fluir
