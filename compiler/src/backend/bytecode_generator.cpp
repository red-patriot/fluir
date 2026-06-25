#include "compiler/backend/bytecode_generator.hpp"

#include <cassert>
#include <cstdint>
#include <format>
#include <ranges>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "compiler/types/traits.hpp"
#include "compiler/utility/scope_guard.hpp"

using fluir::code::Instruction;

namespace fluir {
  void generateCode(Context& ctx, const ast::AST& graph, CodeWriter& writer) {
    return BytecodeGenerator::generate(ctx, graph, writer);
  }

  void BytecodeGenerator::generate(Context& ctx, const ast::AST& graph, CodeWriter& writer) {
    BytecodeGenerator generator{ctx, writer, graph};
    return generator.run();
  }

  void BytecodeGenerator::operator()(const ast::FunctionDecl& func) {
    chunks_.emplace_back();
    current_ = &chunks_.back();
    current_->name = func.name;
    current_->inCount = static_cast<std::uint8_t>(func.parameters.size());
    current_->outCount = func.returnValue ? 1 : 0;

    // TODO: Handle parameters
    auto& [slots, returnCount] = pushScope();
    if (func.returnValue) {
      // If there is a return value, it is reserved at slot 0
      // TODO: Support multiple return values
      slots.insert({func.returnValue->id, 0});
      returnCount = 1;
    }
    // Push parameters in order onto the stack
    for (const auto& param : func.parameters) {
      slots.insert({param.id, slots.size()});
    }
    for (const auto& node : func.statements) {
      recursivelyGenerate(*node);
      if (shouldPopAfter(*node)) {
        // Each top level node will leave a value on the stack,so
        // pop it off iff it was not saved as a new local variable
        emitByte(Instruction::POP);
      }
      // If the value is the return value, discard it
      // HACK: Use a multipop 1 instruction to suppress printing the
      // value until that temp behavior is removed
      if (func.returnValue && func.returnValue->id == node->id()) {
        emitBytes(Instruction::MULTIPOP, 1);
      }
    }
    popScope();

    emitByte(Instruction::RETURN);
  }

  void BytecodeGenerator::generate(const ast::BinaryOp& node) {
    recursivelyGenerate(*node.lhs());
    recursivelyGenerate(*node.rhs());

    // TODO: Handle user-defined ops here
    if (node.lhs()->type() != node.rhs()->type()) {
      diagnostic::emitInternalError("Type mismatch in binary operator");
      return;
    }

    if (const auto type = node.lhs()->type(); type == types::ID_F64) {
      emitFloatOperator(node.op());
    } else if (type == types::ID_I64 || type == types::ID_I32 || type == types::ID_I16 || type == types::ID_I8) {
      emitIntOperator(node.op());
    } else if (type == types::ID_U64 || type == types::ID_U32 || type == types::ID_U16 || type == types::ID_U8) {
      emitUintOperator(node.op());
    } else {
      // TODO: Handle this case better
      diagnostic::emitInternalError("Unknown type encountered");
    }
  }

  void BytecodeGenerator::generate(const ast::UnaryOp& node) {
    constexpr bool IS_UNARY = true;
    recursivelyGenerate(*node.operand());

    if (const auto type = node.operand()->type(); type == types::ID_F64) {
      emitFloatOperator(node.op(), IS_UNARY);
    } else if (type == types::ID_I64 || type == types::ID_I32 || type == types::ID_I16 || type == types::ID_I8) {
      emitIntOperator(node.op(), IS_UNARY);
    } else if (type == types::ID_U64 || type == types::ID_U32 || type == types::ID_U16 || type == types::ID_U8) {
      emitUintOperator(node.op(), IS_UNARY);
    } else {
      // TODO: Handle this case better
      diagnostic::emitInternalError("Unknown type encountered");
    }
  }

  void BytecodeGenerator::generate(const ast::Constant& node) {
    auto type = node.type();
    size_t constant;
    if (type == types::ID_F64) {
      constant = addConstant(node.f64());
    } else if (type == types::ID_I8) {
      constant = addConstant(node.i8());
    } else if (type == types::ID_I16) {
      constant = addConstant(node.i16());
    } else if (type == types::ID_I32) {
      constant = addConstant(node.i32());
    } else if (type == types::ID_I64) {
      constant = addConstant(node.i64());
    } else if (type == types::ID_U8) {
      constant = addConstant(node.u8());
    } else if (type == types::ID_U16) {
      constant = addConstant(node.u16());
    } else if (type == types::ID_U32) {
      constant = addConstant(node.u32());
    } else if (type == types::ID_U64) {
      constant = addConstant(node.u64());
    } else {
      diagnostic::emitInternalError("Unknown constant type encountered.");
      return;
    }
    if (constant <= UINT8_MAX) {
      emitBytes(Instruction::PUSH, static_cast<std::uint8_t>(constant));
    } else {
      emitByte(Instruction::QUAD_PUSH);
      emitLongOperand(constant);
    }
  }

  void BytecodeGenerator::generate(const ast::Cast& cast) {
    recursivelyGenerate(*cast.operand());

    // TODO: Handle user-defined casts here
    auto operandType = cast.operand()->type();
    auto targetType = cast.type();
    if (types::isIntegral(targetType)) {
      if (types::isSigned(targetType)) {
        if (types::isIntegral(operandType)) {
          if (types::isSigned(operandType)) {
            emitWidthCast(operandType, targetType);
          } else {
            emitBytes(Instruction::CAST_UI, types::widthOf(targetType));
          }
        } else {
          emitBytes(Instruction::CAST_FI, types::widthOf(targetType));
        }
      } else {
        if (types::isIntegral(operandType)) {
          if (types::isSigned(operandType)) {
            emitBytes(Instruction::CAST_IU, types::widthOf(targetType));
          } else {
            emitWidthCast(operandType, targetType);
          }
        } else {
          emitBytes(Instruction::CAST_FU, types::widthOf(targetType));
        }
      }
    } else {
      if (types::isIntegral(operandType)) {
        if (types::isSigned(operandType)) {
          emitByte(Instruction::CAST_IF);
        } else {
          emitByte(Instruction::CAST_UF);
        }
      } else {
        // No-op F64->F64
      }
    }
  }

  void BytecodeGenerator::generate(const ast::LocalWrite& write) {
    recursivelyGenerate(*write.child());

    auto& [slots, returnCount] = scopes_.top();
    if (slots.contains(write.variable())) {
      // LocalWrite is updating an existing value.
      const auto slot = static_cast<std::uint8_t>(slots.at(write.variable()));
      emitBytes(Instruction::SET_VAL, slot);
    } else {
      // This LocalWrite is initializing a new value, so make a new slot for it
      const auto nextIndex = slots.size();
      if (nextIndex >= std::numeric_limits<std::uint8_t>::max()) {
        // TODO: Increase this limit
        diagnostic::emitInternalError(std::format("Too many local variables defined. Only {} variables allowed.",
                                                  std::numeric_limits<std::uint8_t>::max()));
      }
      slots.insert({write.variable(), nextIndex});
    }
  }
  void BytecodeGenerator::generate(const ast::LocalRead& read) {
    const auto& [slots, returnCount] = scopes_.top();
    if (!slots.contains(read.variable())) {
      // This shouldn't happen because it should be caught in type checking
      diagnostic::emitInternalError(fmt::format(
        "Expected variable {}, read by node ({}) not found.", read.variable(), fmt::join(read.fullId(), ":")));
    }
    const auto slot = slots.at(read.variable());
    emitBytes(Instruction::GET_VAL, static_cast<uint8_t>(slot));
  }

  void BytecodeGenerator::generate(const ast::Call& call) {
    auto targetType = ctx_.symbolTable.getFunctionType(call.target());
    if (!targetType) {
      diagnostic::emitInternalError(fmt::format("'{}' is not a function", call.target()));
    }

    if (targetType->returnType()) {
      // Reserve space for the return value of the function if it has a return value
      emitBytes(Instruction::RESERVE, 1);
    }

    for (auto& arg : call.arguments()) {
      recursivelyGenerate(*arg);
    }

    if (ctx_.symbolTable.isMagicBuiltin(targetType)) {
      // Emit special instructions for a builtin
      auto call_index = addConstant(call.target());
      emitByte(Instruction::DYN_CALL);
      emitLongOperand(call_index);
    } else {
      // Find the function to execute
      if (!functionIndices_.contains(call.target())) {
        diagnostic::emitInternalError(std::format("'{}' is not found to call", call.target()));
      }
      const auto index = functionIndices_.at(call.target());
      emitByte(Instruction::CALL);
      emitLongOperand(index);
    }
  }

  BytecodeGenerator::BytecodeGenerator(Context& ctx, CodeWriter& writer, const ast::AST& graph) :
    ctx_(ctx), graph_(graph), writer_(writer) { }

  void BytecodeGenerator::emitByte(std::uint8_t byte) { current_->code.push_back(byte); }

  void BytecodeGenerator::emitBytes(std::uint8_t byte1, std::uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
  }
  void BytecodeGenerator::emitLongOperand(std::uint64_t arg) {
    static constexpr int BYTE_SIZE = 8;
    static constexpr int SHIFT = BYTE_SIZE * 3;
    for (int i = 0; i != 4; ++i) {
      // Consume the top 8 bits of the operand and emit them one by one to write into the bytecode.
      const auto byte = static_cast<std::uint8_t>(arg >> SHIFT);
      emitByte(byte);
      arg <<= BYTE_SIZE;
    }
  }

  size_t BytecodeGenerator::addConstant(be::Constant value) {
    if (auto found = std::ranges::find(constants_, value); found != constants_.end()) {
      return found - constants_.begin();
    }
    constants_.emplace_back(std::move(value));
    return constants_.size() - 1;
  }

  void BytecodeGenerator::run() {
    for (const auto& [index, declaration] : std::views::enumerate(graph_.declarations)) {
      // Track the indices of each function to manage calls
      functionIndices_.insert({declaration.name, index});
    }

    for (const auto& declaration : graph_.declarations) {
      (*this)(declaration);
    }

    header_.major = ctx_.version.major;
    header_.minor = ctx_.version.minor;
    header_.patch = ctx_.version.patch;

    writer_.writeHeader(header_);
    writer_.writeConstants(constants_);
    for (const auto& chunk : chunks_) {
      writer_.writeChunk(chunk);
    }
  }

  void BytecodeGenerator::recursivelyGenerate(const ast::Node& node) {
    switch (node.kind()) {
      case ast::NodeKind::BinaryOperator:
        return generate(*node.as<ast::BinaryOp>());
      case ast::NodeKind::UnaryOperator:
        return generate(*node.as<ast::UnaryOp>());
      case ast::NodeKind::Constant:
        return generate(*node.as<ast::Constant>());
      case ast::NodeKind::Cast:
        return generate(*node.as<ast::Cast>());
      case ast::NodeKind::LocalWrite:
        return generate(*node.as<ast::LocalWrite>());
      case ast::NodeKind::LocalRead:
        return generate(*node.as<ast::LocalRead>());
      case ast::NodeKind::Call:
        return generate(*node.as<ast::Call>());
    }
  }

  BytecodeGenerator::Scope& BytecodeGenerator::pushScope() {
    scopes_.emplace();
    return scopes_.top();
  }
  void BytecodeGenerator::popScope() {
    auto& currentScope = scopes_.top();
    // Clean up the local variables from this scope before popping it
    if (const auto toPop = static_cast<std::uint8_t>(currentScope.slots.size() - currentScope.returnCount); toPop > 0) {
      emitBytes(Instruction::MULTIPOP, toPop);
    }
    scopes_.pop();
  }

  bool BytecodeGenerator::shouldPopAfter(const ast::Node& node) {
    const auto& [slots, _] = scopes_.top();
    if (slots.contains(node.id())) {
      return false;
    }

    if (node.is<ast::Call>()) {
      const auto& call = node.as<ast::Call>();
      const auto& targetType = ctx_.symbolTable.getFunctionType(call->target());
      return targetType->returnType().has_value();
    }

    return true;
  }

  void BytecodeGenerator::emitFloatOperator(const Operator op, bool unary) {
    switch (op) {
      case Operator::PLUS:
        if (unary) {
          // No instruction emitted
        } else {
          emitByte(Instruction::F64_ADD);
        }
        break;
      case Operator::MINUS:
        if (unary) {
          emitByte(Instruction::F64_NEG);
        } else {
          emitByte(Instruction::F64_SUB);
        }
        break;
      case Operator::STAR:
        emitByte(Instruction::F64_MUL);
        break;
      case Operator::SLASH:
        emitByte(Instruction::F64_DIV);
        break;
      case Operator::PLUS_PLUS:
        emitByte(Instruction::F64_INC);
        break;
      case Operator::MINUS_MINUS:
        emitByte(Instruction::F64_DEC);
        break;
      case Operator::UNKNOWN:
        // TODO: Handle this better
      default:
        // TODO-BOOLEAN
        diagnostic::emitInternalError("Unknown operator encountered. Expected one of +, -, *, /");
        break;
    }
  }
  void BytecodeGenerator::emitIntOperator(const Operator op, bool unary) {
    switch (op) {
      case Operator::PLUS:
        if (unary) {
          // No instruction emitted
        } else {
          emitByte(Instruction::I64_ADD);
        }
        break;
      case Operator::MINUS:
        if (unary) {
          emitByte(Instruction::I64_NEG);
          break;
        }
        emitByte(Instruction::I64_SUB);
        break;
      case Operator::STAR:
        emitByte(Instruction::I64_MUL);
        break;
      case Operator::SLASH:
        emitByte(Instruction::I64_DIV);
        break;
      case Operator::PLUS_PLUS:
        emitByte(Instruction::I64_INC);
        break;
      case Operator::MINUS_MINUS:
        emitByte(Instruction::I64_DEC);
        break;
      case Operator::UNKNOWN:
        // TODO: Handle this better
      default:
        // TODO-BOOLEAN
        diagnostic::emitInternalError("Unknown operator encountered. Expected one of +, -, *, /");
        break;
    }
  }
  void BytecodeGenerator::emitUintOperator(const Operator op, bool unary) {
    switch (op) {
      case Operator::PLUS:
        if (unary) {
          // No instruction emitted
          break;
        }
        emitByte(Instruction::U64_ADD);
        break;
      case Operator::MINUS:
        emitByte(Instruction::U64_SUB);
        break;
      case Operator::STAR:
        emitByte(Instruction::U64_MUL);
        break;
      case Operator::SLASH:
        emitByte(Instruction::U64_DIV);
        break;
      case Operator::PLUS_PLUS:
        emitByte(Instruction::U64_INC);
        break;
      case Operator::MINUS_MINUS:
        emitByte(Instruction::U64_DEC);
        break;
      case Operator::UNKNOWN:
        // TODO: Handle this better
      default:
        // TODO-BOOLEAN
        diagnostic::emitInternalError("Unknown operator encountered. Expected one of +, -, *, /");
        break;
    }
  }

  void BytecodeGenerator::emitWidthCast(types::TypeID sourceType, types::TypeID targetType) {
    if (types::widthOf(sourceType) == types::widthOf(targetType)) {
      return;
    }
    emitBytes(Instruction::CAST_WIDTH, types::widthOf(targetType));
  }
}  // namespace fluir
