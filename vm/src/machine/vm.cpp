#include "../../include/vm/machine/vm.hpp"

#include <algorithm>
#include <cassert>
#include <format>  // Use format in VM instead of fmt to reduce dependencies of the runtime
#include <functional>
#include <iostream>

#include "vm/debug.hpp"
#include "vm/exceptions.hpp"
#include "vm/utility/narrow_widen.hpp"
#include "vm/utility/operations.hpp"

namespace fluir {
  template <typename Op>
  void VirtualMachine::floatBinary() {
    double rhs = stackTop().asF64();
    popStack();
    double lhs = stackTop().asF64();
    popStack();
    pushStack(code::Value{Op{}(lhs, rhs)});
  }
  template <typename Op>
  void VirtualMachine::floatUnary() {
    double operand = stackTop().asF64();
    popStack();
    pushStack(code::Value(Op{}(operand)));
  }
  template <typename Op>
  void VirtualMachine::intBinary() {
    code::PrimitiveType typeR, typeL;
    code::I64 rhs = utility::widenI(stackTop(), typeR);
    popStack();
    code::I64 lhs = utility::widenI(stackTop(), typeL);
    popStack();
    pushStack(utility::narrowI(Op{}(lhs, rhs), std::max(utility::widthof(typeR), utility::widthof(typeL))));
  }
  template <typename Op>
  void VirtualMachine::intUnary() {
    code::PrimitiveType type;
    code::I64 operand = utility::widenI(stackTop(), type);
    popStack();
    pushStack(utility::narrowI(Op{}(operand), utility::widthof(type)));
  }
  template <typename Op>
  void VirtualMachine::uintBinary() {
    code::PrimitiveType typeR, typeL;
    code::U64 rhs = utility::widenU(stackTop(), typeR);
    popStack();
    code::U64 lhs = utility::widenU(stackTop(), typeL);
    popStack();
    pushStack(utility::narrowU(Op{}(lhs, rhs), std::max(utility::widthof(typeR), utility::widthof(typeL))));
  }
  template <typename Op>
  void VirtualMachine::uintUnary() {
    code::PrimitiveType type;
    code::U64 operand = utility::widenU(stackTop(), type);
    popStack();
    pushStack(utility::narrowU(Op{}(operand), utility::widthof(type)));
  }

  VirtualMachine::VirtualMachine(NativeFunctionsMap natives) : natives_(std::move(natives)) { }

  ExecResult VirtualMachine::execute(code::ByteCode const* code) {
    code_ = code;
    // Reset the internal state
    init();

    try {
      return run();
    } catch (const DivideByZeroError& e) {
      std::cerr << e.what() << std::endl;
      return ExecResult::ERROR_DIVIDE_BY_ZERO;
    } catch (const VirtualMachineError& e) {
      std::cerr << e.what() << std::endl;
      return ExecResult::ERROR;
    } catch (const std::exception& e) {
      std::cerr << "An unexpected error occurred: \n" << e.what() << std::endl;
      return ExecResult::ERROR;
    } catch (...) {
      std::cerr << "VM PANIC" << std::endl;
      return ExecResult::ERROR;
    }
  }

  void VirtualMachine::init() {
    // Reset the stack
    if (!stack_) {
      stack_ = std::make_unique<Stack>();
    }
    frames_.reserve(FUNCTION_DEPTH);
    std::ranges::fill(*stack_, code::Value{});

    auto mainIt = std::ranges::find(code_->chunks, "main", &code::Chunk::name);
    if (mainIt == code_->chunks.end()) {
      throw VirtualMachineError{"No 'main' function found in bytecode"};
    }

    createFlStartup(static_cast<size_t>(std::distance(code_->chunks.begin(), mainIt)));
    CallFrame initFrame{
      .chunk = &flStartup_,
      .returnAddress = ip_,
      .basePtr = stack_->data(),
      .stackEnd = stack_->data(),
    };
    ip_ = flStartup_.code.data();
    frames_.emplace_back(initFrame);
    currentFrame_ = &frames_.back();
  }

  ExecResult VirtualMachine::run() {
#define FLUIR_READ_BYTE() *ip_++

    using enum code::Instruction;
    for (;;) {
      std::uint8_t instruction = FLUIR_READ_BYTE();
#if FLUIR_ENABLE_DEBUGGING
      debug::printInstruction(instruction);
      debug::printStack(std::span{stack_->data(), currentFrame_->stackEnd});
#endif
      switch (instruction) {
        case PUSH:
          {
            uint8_t index = FLUIR_READ_BYTE();
            const code::Value& val = code_->constants[index];
            if (stackSize() >= STACK_LIMIT) {
              return ExecResult::ERROR;
            }
            pushStack(val);
            break;
          }
        case QUAD_PUSH:
          {
            uint64_t index = readQuadWord();
            const code::Value& val = code_->constants[index];
            if (stackSize() >= STACK_LIMIT) {
              return ExecResult::ERROR;
            }
            pushStack(val);
            break;
          }
        case GET_VAL:
          {
            const auto index = FLUIR_READ_BYTE();
            if (stackSize() >= STACK_LIMIT) {
              return ExecResult::ERROR;
            }
            pushStack(currentFrame_->basePtr[index]);
            break;
          }
        case SET_VAL:
          {
            const auto index = FLUIR_READ_BYTE();
            currentFrame_->basePtr[index] = stackTop();
            break;
          }
        case F64_ADD:
          floatBinary<std::plus<code::F64>>();
          break;
        case F64_SUB:
          floatBinary<std::minus<code::F64>>();
          break;
        case F64_MUL:
          floatBinary<std::multiplies<code::F64>>();
          break;
        case F64_DIV:
          floatBinary<std::divides<code::F64>>();
          break;
        case F64_NEG:
          floatUnary<std::negate<code::F64>>();
          break;
        case F64_INC:
          floatUnary<utility::increment<code::F64>>();
          break;
        case F64_DEC:
          floatUnary<utility::decrement<code::F64>>();
          break;
        case I64_ADD:
          intBinary<std::plus<code::I64>>();
          break;
        case I64_SUB:
          intBinary<std::minus<code::I64>>();
          break;
        case I64_MUL:
          intBinary<std::multiplies<code::I64>>();
          break;
        case I64_DIV:
          intBinary<utility::checkedDivide<code::I64>>();
          break;
        case I64_INC:
          intUnary<utility::increment<code::I64>>();
          break;
        case I64_DEC:
          intUnary<utility::decrement<code::I64>>();
          break;
        case I64_NEG:
          intUnary<std::negate<code::I64>>();
          break;
        case U64_ADD:
          uintBinary<std::plus<code::U64>>();
          break;
        case U64_SUB:
          uintBinary<std::minus<code::U64>>();
          break;
        case U64_MUL:
          uintBinary<std::multiplies<code::U64>>();
          break;
        case U64_DIV:
          uintBinary<utility::checkedDivide<code::U64>>();
          break;
        case U64_INC:
          uintUnary<utility::increment<code::U64>>();
          break;
        case U64_DEC:
          uintUnary<utility::decrement<code::U64>>();
          break;
        case CAST_IU:
          {
            auto width = static_cast<code::NumericWidth>(FLUIR_READ_BYTE());
            auto toCast = stackTop();
            popStack();
            code::PrimitiveType _;
            auto widened = utility::widenI(toCast, _);
            auto casted = static_cast<code::U64>(widened);

            pushStack(utility::narrowU(casted, width));
          }
          break;
        case CAST_UI:
          {
            auto width = static_cast<code::NumericWidth>(FLUIR_READ_BYTE());
            auto toCast = stackTop();
            popStack();
            code::PrimitiveType _;
            auto widened = utility::widenU(toCast, _);
            auto casted = static_cast<code::I64>(widened);
            pushStack(utility::narrowI(casted, width));
          }
          break;
        case CAST_IF:
          {
            auto toCast = stackTop();
            popStack();
            code::PrimitiveType _;
            auto widened = utility::widenI(toCast, _);
            auto casted = static_cast<code::F64>(widened);
            pushStack(code::Value{casted});
          }
          break;
        case CAST_FI:
          {
            auto width = static_cast<code::NumericWidth>(FLUIR_READ_BYTE());
            auto toCast = stackTop();
            popStack();
            auto casted = static_cast<code::I64>(toCast.asF64());
            pushStack(utility::narrowI(casted, width));
          }
          break;
        case CAST_UF:
          {
            auto toCast = stackTop();
            popStack();
            code::PrimitiveType _;
            auto widened = utility::widenU(toCast, _);
            auto casted = static_cast<code::F64>(widened);
            pushStack(code::Value{casted});
          }
          break;
        case CAST_FU:
          {
            auto width = static_cast<code::NumericWidth>(FLUIR_READ_BYTE());
            auto toCast = stackTop();
            popStack();
            auto casted = static_cast<code::U64>(toCast.asF64());
            pushStack(utility::narrowU(casted, width));
          }
          break;
        case CAST_WIDTH:
          {
            auto width = static_cast<code::NumericWidth>(FLUIR_READ_BYTE());
            auto toCast = stackTop();
            popStack();
            code::PrimitiveType _;
            switch (toCast.type()) {
              case code::PrimitiveType::I8:
              case code::PrimitiveType::I16:
              case code::PrimitiveType::I32:
              case code::PrimitiveType::I64:
                pushStack(utility::narrowI(utility::widenI(toCast, _), width));
                break;
              case code::PrimitiveType::U8:
              case code::PrimitiveType::U16:
              case code::PrimitiveType::U32:
              case code::PrimitiveType::U64:
                pushStack(utility::narrowU(utility::widenU(toCast, _), width));
                break;
              default:
                throw VirtualMachineError{"CAST_WIDTH EXPECTS AN INT OR UINT"};
            }
          }
          break;
        case POP:
          popStack();
          break;
        case MULTIPOP:
          {
            auto count = FLUIR_READ_BYTE();
            for (size_t i = 0; i != count; ++i) {
              popStack();
            }
            break;
          }
        case EXIT:
          goto afterLoop;
        case RESERVE:
          {
            auto count = FLUIR_READ_BYTE();
            for (std::uint8_t i = 0; i != count; ++i) {
              pushStack(code::Value{});
            }
            break;
          }
        case CALL:
          {
            auto calleeIndex = readQuadWord();
            auto callee = &code_->chunks.at(calleeIndex);
            auto offset = callee->inCount + callee->outCount;
            auto basePtr = currentFrame_->stackEnd - offset;
            initCall(callee, basePtr);
            break;
          }
        case DYN_CALL:
          {
            auto calleeIndex = readQuadWord();
            const auto& val = code_->constants.at(calleeIndex).asStr();
            std::string_view name{val.chars.get(), val.size};
            const auto callee = natives_.find(name);
            if (callee == natives_.end()) {
              throw VirtualMachineError{std::format("{} NOT FOUND", name)};
            }
            callee->second(*currentFrame_);
          }
          break;
        case RETURN:
          {
            auto returnAddress = currentFrame_->returnAddress;
            ip_ = returnAddress;
            frames_.pop_back();
            currentFrame_ = &frames_.back();
          }
          break;
        default:
          return ExecResult::ERROR;
      }
    }
  afterLoop:
    return ExecResult::SUCCESS;
  }

  void VirtualMachine::createFlStartup(std::uint64_t index) {
    using enum code::Instruction;
    auto index0 = static_cast<std::uint8_t>(index >> 24);
    auto index1 = static_cast<std::uint8_t>(index >> 16);
    auto index2 = static_cast<std::uint8_t>(index >> 8);
    auto index3 = static_cast<std::uint8_t>(index);

    flStartup_ = code::Chunk{.name = "_fl_start",
                             .code =
                               {
                                 CALL,
                                 index0,
                                 index1,
                                 index2,
                                 index3,
                                 EXIT,
                               },
                             .inCount = 0,
                             .outCount = 0};
  }

  code::Value& VirtualMachine::stackTop() { return *(currentFrame_->stackEnd - 1); }
  void VirtualMachine::popStack() { --currentFrame_->stackEnd; }
  void VirtualMachine::pushStack(code::Value value) {
    (*currentFrame_->stackEnd) = std::move(value);
    ++currentFrame_->stackEnd;
  }

  std::uint8_t VirtualMachine::readByte() { return *ip_++; }

  std::uint64_t VirtualMachine::readQuadWord() {
    std::uint64_t word = 0;
    for (int i = 0; i != 4; ++i) {
      const auto next = readByte();
      word <<= 8;
      word |= next;
    }

    return word;
  }

  void VirtualMachine::initCall(code::Chunk const* callee, code::Value* basePtr) {
    // Offset the top of the current frame by the number of parameters
    // passed, since the callee will pop them off itself as it runs
    currentFrame_->stackEnd -= callee->inCount;
    CallFrame frame{
      .chunk = callee,
      .returnAddress = ip_,
      .basePtr = basePtr,
      .stackEnd = basePtr + callee->inCount + callee->outCount,
    };
    ip_ = callee->code.data();
    frames_.emplace_back(std::move(frame));
    currentFrame_ = &frames_.back();
  }
}  // namespace fluir
