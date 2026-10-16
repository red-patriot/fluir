#include "compiler/backend/bytecode_generator.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <format>
#include <ranges>

#include <fmt/format.h>

#include "compiler/types/traits.hpp"
#include "compiler/utility/scope_guard.hpp"

using fluir::code::Instruction;

namespace fluir {
  namespace {
    bool shouldInvert(const Operator op) {
      switch (op) {
        case Operator::GREATER:        // lhs > rhs => rhs < lhs
        case Operator::GREATER_EQUAL:  // lhs >= rhs => rhs <= lhs
          return true;
        default:
          return false;
      }
    }
  }  // namespace

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

    std::vector<WithID<types::TypeID>> outputs;
    if (func.returnValue) {
      auto returnType = ctx_.symbolTable.getTypeID(func.returnValue->typeName);
      outputs.emplace_back(func.returnValue->id, returnType);
    }
    std::vector<WithID<types::TypeID>> inputs;
    std::ranges::transform(func.parameters, std::back_inserter(inputs), [this](const auto& p) -> WithID<types::TypeID> {
      auto paramType = ctx_.symbolTable.getTypeID(p.typeName);
      return {p.id, paramType};
    });

    {
      auto frame = be::BytecodeFrame(ctx_, inputs, outputs, current_->code);
      auto scope = frame.pushScope();
      const auto lastFrame = frame_;
      const auto lastScope = currentScope_;
      frame_ = &frame;
      currentScope_ = &scope;
      FLUIR_SCOPE_EXIT {
        currentScope_ = lastScope;
        frame_ = lastFrame;
      };

      generate(func.statements, inputs, outputs);
    }
    emitByte(Instruction::RETURN);
  }

  void BytecodeGenerator::generate(const ast::BinaryOp& node) {
    if (shouldInvert(node.op())) {
      // Some operators are implemented with their inverse in the VM,
      // so generate their operands in the reverse order to put them
      // on the right order on the stack
      generateDependency(node.rhs());
      generateDependency(node.lhs());
    } else {
      generateDependency(node.lhs());
      generateDependency(node.rhs());
    }

    // TODO: Handle user-defined ops here
    if (node.lhs().type() != node.rhs().type()) {
      diagnostic::emitInternalError("Type mismatch in binary operator");
      return;
    }

    const auto type = node.lhs().type();
    switch (type) {
      case types::ID_F64:
        emitFloatOperator(node.op());
        break;
      case types::ID_I8:
      case types::ID_I16:
      case types::ID_I32:
      case types::ID_I64:
        emitIntOperator(node.op());
        break;
      case types::ID_U8:
      case types::ID_U16:
      case types::ID_U32:
      case types::ID_U64:
        emitUintOperator(node.op());
        break;
      case types::ID_BOOL:
        emitBoolOperator(node.op());
        break;
      default:
        // TODO: Handle this case better
        diagnostic::emitInternalError("Unknown type encountered");
    }
  }

  void BytecodeGenerator::generate(const ast::UnaryOp& node) {
    constexpr bool IS_UNARY = true;
    generateDependency(node.operand());

    const auto type = node.operand().type();
    switch (type) {
      case types::ID_F64:
        emitFloatOperator(node.op(), IS_UNARY);
        break;
      case types::ID_I64:
      case types::ID_I32:
      case types::ID_I16:
      case types::ID_I8:
        emitIntOperator(node.op(), IS_UNARY);
        break;
      case types::ID_U64:
      case types::ID_U32:
      case types::ID_U16:
      case types::ID_U8:
        emitUintOperator(node.op(), IS_UNARY);
        break;
      case types::ID_BOOL:
        emitBoolOperator(node.op(), IS_UNARY);
        break;
      default:
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
    } else if (type == types::ID_BOOL) {
      constant = addConstant(node.boolean());
    } else {
      diagnostic::emitInternalError("Unknown constant type encountered.");
    }
    if (constant <= UINT8_MAX) {
      emitBytes(Instruction::PUSH, static_cast<std::uint8_t>(constant));
    } else {
      emitByte(Instruction::QUAD_PUSH);
      emitLongOperand(constant);
    }
  }

  void BytecodeGenerator::generate(const ast::Cast& cast) {
    generateDependency(cast.operand());
    // TODO: Handle user-defined casts here
    auto operandType = cast.operand().type();
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
    generateDependency(write.child());

    auto slot = addLocalSlot(write.variable(), write.storedType());
    emitBytes(Instruction::SET_VAL, slot);
  }

  void BytecodeGenerator::generate(const ast::LocalRead& read) {
    if (!currentScope_->contains(read.variable())) {
      // This shouldn't happen because it should be caught in type checking
      diagnostic::emitInternalError(fmt::format(
        "Expected variable {}, read by node ({}) not found.", read.variable(), fmt::join(read.fullId(), ":")));
    }
    const auto slot = currentScope_->getLocalSlot(read.variable());
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
      generateDependency(arg);
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

  void BytecodeGenerator::generate(const ast::Conditional& conditional) {
    /*
     *       [condition]
     *       JUMP_IF_FALSE   ------|
     *         // THEN BODY        |
     *         MULTIPOP (inputs)   |
     *  |--- JUMP                  |
     *  |      // ELSE BODY  <-----|
     *  |      MULTIPOP (inputs)
     *  |--> // REST...
     */
    std::vector<WithID<types::TypeID>> inputs;
    inputs.reserve(conditional.inputs().size());
    for (const auto& [dependency, innerId] : conditional.inputs()) {
      generateDependency(dependency);
      inputs.emplace_back(innerId, dependency.type());
    }

    generateDependency(conditional.condition());
    inputs.emplace_back(1, types::ID_BOOL);  // Inner ID of the condition

    auto lastScope = currentScope_;
    auto scope = currentScope_->pushScope();
    currentScope_ = &scope;
    FLUIR_SCOPE_EXIT { currentScope_ = lastScope; };
    for (const auto& [id, type] : inputs | std::ranges::views::reverse) {
      auto slot = addLocalSlot(id, type);
      emitBytes(Instruction::SET_VAL, slot);
    }
    // Read the condition back to the stack to use it
    emitBytes(Instruction::GET_VAL, currentScope_->getLocalSlot(1));

    std::vector<WithID<types::TypeID>> outputs;
    if (conditional.outputs().size() == 0) {
    } else if (conditional.outputs().size() == 1) {
      outputs.emplace_back(conditional.outputs().front(), conditional.type());
    } else {
      auto product = ctx_.symbolTable.getType(conditional.type())->as<types::Product>();
      for (const auto& [idx, id] : std::ranges::views::enumerate(conditional.outputs())) {
        outputs.emplace_back(id, product->at(idx));
        addLocalSlot(id, product->at(idx));
      }
    }

    auto thenJump = emitJump(Instruction::JUMP_IF_FALSE);
    generate(conditional.thenBody(), inputs, outputs);
    auto elseJump = emitJump(Instruction::JUMP_BY);
    patchJump(thenJump, current_->code.size());

    generate(conditional.elseBody(), inputs, outputs);
    patchJump(elseJump, current_->code.size());

    // Push the outputs onto the stack
    for (const auto& id : conditional.outputs()) {
      emitBytes(Instruction::GET_VAL, currentScope_->getLocalSlot(id));
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

  size_t BytecodeGenerator::emitJump(code::Instruction instruction) {
    emitByte(instruction);
    auto jumpIndex = current_->code.size();
    emitLongOperand(0xffffffff);  // Emit a placeholder for the jump operand
    return jumpIndex;
  }
  void BytecodeGenerator::patchJump(size_t jumpIndex, size_t targetIndex) {
    static constexpr int BYTE_SIZE = 8;
    static constexpr int SHIFT = BYTE_SIZE * 3;

    auto offset = targetIndex - (jumpIndex + 4);
    for (int i = 0; i != 4; ++i) {
      // Consume the top 8 bits of the offset and emit them one by one to write into the bytecode.
      const auto byte = static_cast<std::uint8_t>(offset >> SHIFT);
      current_->code[jumpIndex + i] = byte;
      offset <<= BYTE_SIZE;
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

  void BytecodeGenerator::generate(const ast::DataFlowGraph& subgraph,
                                   const std::vector<WithID<types::TypeID>>& inputs,
                                   const std::vector<WithID<types::TypeID>>& outputs) {
    auto outputCount = outputs.size();
    if (outputCount >= UINT8_MAX) {
      diagnostic::emitInternalError(
        fmt::format("Too many outputs from subgraph. Found {}, only up to {} allowed.", outputCount, UINT8_MAX));
    }

    for (const auto& [id, type] : outputs) {
      addLocalSlot(id, type);
    }
    for (const auto& [id, type] : inputs) {
      addLocalSlot(id, type);
    }
    for (const auto& node : subgraph) {
      generateTopLevel(*node);
    }
  }

  void BytecodeGenerator::generateDependency(const ast::Dependency& dependency) { doGenerate(dependency.get()); }

  void BytecodeGenerator::generateTopLevel(const ast::Node& node) {
    doGenerate(node);
    // TODO: Handle sizes generically based by type
    const auto* type = ctx_.symbolTable.getType(node.type());
    if (auto slotCount = type ? be::slotsOf(*type) : 0; slotCount == 0) {
      // No emit for no slots
    } else if (slotCount == 1) {
      emitByte(Instruction::POP);
    } else {
      emitBytes(Instruction::MULTIPOP, slotCount);
    }
  }

  void BytecodeGenerator::doGenerate(const ast::Node& node) {
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
      case ast::NodeKind::Conditional:
        return generate(*node.as<ast::Conditional>());
    }
    diagnostic::emitInternalError("Unknown node kind encountered");
  }

  uint8_t BytecodeGenerator::addLocalSlot(const ID& id, types::TypeID type) {
    const auto* ptr = ctx_.symbolTable.getType(type);
    if (!ptr || !currentScope_) {
      diagnostic::emitInternalError("Encountered unexpected null");
    }

    return currentScope_->addLocalSlot(id, *ptr);
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
      case Operator::LESS:
      case Operator::GREATER:  // Operands are inverted for this operand, same instruction is emitted
        emitByte(Instruction::F64_LT);
        break;
      case Operator::LESS_EQUAL:
      case Operator::GREATER_EQUAL:  // Operands are inverted for this operand, same instruction is emitted
        emitByte(Instruction::F64_LE);
        break;
      case Operator::EQUAL_EQUAL:
        emitByte(Instruction::EQ);
        break;
      case Operator::BANG_EQUAL:
        emitBytes(Instruction::EQ, Instruction::NOT);
        break;
      case Operator::UNKNOWN:
        // TODO: Handle this better
      default:
        // TODO-BOOLEAN
        diagnostic::emitInternalError("Unknown operator encountered.");
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
      case Operator::LESS:
      case Operator::GREATER:  // Operands are inverted for this operand, same instruction is emitted
        emitByte(Instruction::I64_LT);
        break;
      case Operator::LESS_EQUAL:
      case Operator::GREATER_EQUAL:  // Operands are inverted for this operand, same instruction is emitted
        emitByte(Instruction::I64_LE);
        break;
      case Operator::EQUAL_EQUAL:
        emitByte(Instruction::EQ);
        break;
      case Operator::BANG_EQUAL:
        emitBytes(Instruction::EQ, Instruction::NOT);
        break;
      case Operator::UNKNOWN:
        // TODO: Handle this better
      default:
        // TODO-BOOLEAN
        diagnostic::emitInternalError("Unknown operator encountered.");
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
      case Operator::LESS:
      case Operator::GREATER:  // Operands are inverted for this operand, same instruction is emitted
        emitByte(Instruction::U64_LT);
        break;
      case Operator::LESS_EQUAL:
      case Operator::GREATER_EQUAL:  // Operands are inverted for this operand, same instruction is emitted
        emitByte(Instruction::U64_LE);
        break;
      case Operator::EQUAL_EQUAL:
        emitByte(Instruction::EQ);
        break;
      case Operator::BANG_EQUAL:
        emitBytes(Instruction::EQ, Instruction::NOT);
        break;
      case Operator::UNKNOWN:
        // TODO: Handle this better
      default:
        // TODO-BOOLEAN
        diagnostic::emitInternalError("Unknown operator encountered.");
        break;
    }
  }

  void BytecodeGenerator::emitBoolOperator(const Operator op, [[maybe_unused]] bool unary) {
    switch (op) {
      case Operator::AND_AND:
        emitByte(Instruction::AND);
        break;
      case Operator::BAR_BAR:
        emitByte(Instruction::OR);
        break;
      case Operator::BANG:
        emitByte(Instruction::NOT);
        break;
      case Operator::EQUAL_EQUAL:
        emitByte(Instruction::EQ);
        break;
      case Operator::BANG_EQUAL:
        emitBytes(Instruction::EQ, Instruction::NOT);
        break;
      default:
        // TODO: Handle this better
        diagnostic::emitInternalError("Unsupported operator for type Bool");
    }
  }

  void BytecodeGenerator::emitWidthCast(types::TypeID sourceType, types::TypeID targetType) {
    if (types::widthOf(sourceType) == types::widthOf(targetType)) {
      return;
    }
    emitBytes(Instruction::CAST_WIDTH, types::widthOf(targetType));
  }
}  // namespace fluir
