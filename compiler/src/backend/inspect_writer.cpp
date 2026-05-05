#include "compiler/backend/inspect_writer.hpp"

#include <format>
#include <string>

#include "fmt/format.h"

namespace fluir {
  namespace {
#define STRINGIFY(i) #i
#define FLUIR_INSTRUCTION_TO_STR(inst) STRINGIFY(I##inst),
    std::string instructionNames[] = {FLUIR_CODE_INSTRUCTIONS(FLUIR_INSTRUCTION_TO_STR)};
#undef FLUIR_INSTRUCTION_TO_STR
#undef STRINGIFY

    struct ConstantWriter {
      std::string operator()(double d) const { return fmt::format("VF64 {:.12f}", d); }
      std::string operator()(int8_t i) const { return fmt::format("VI8  x{:X}", i); }
      std::string operator()(int16_t i) const { return fmt::format("VI16 x{:X}", i); }
      std::string operator()(int32_t i) const { return fmt::format("VI32 x{:X}", i); }
      std::string operator()(int64_t i) const { return fmt::format("VI64 x{:X}", i); }
      std::string operator()(uint8_t u) const { return fmt::format("VU8  x{:X}", u); }
      std::string operator()(uint16_t u) const { return fmt::format("VU16 x{:X}", u); }
      std::string operator()(uint32_t u) const { return fmt::format("VU32 x{:X}", u); }
      std::string operator()(uint64_t u) const { return fmt::format("VU64 x{:X}", u); }
    };
  }  // namespace

  void InspectWriter::writeHeader(const code::Header& header) {
    os_ << fmt::format(
      "I{:0>2X}{:0>2X}{:0>2X}{:0>16X}\n", header.major, header.minor, header.patch, header.entryOffset);
  }

  void InspectWriter::writeConstants(const be::ConstantsArray& constants) {
    os_ << fmt::format("CONSTANTS x{:X}\n", constants.size());
    [[maybe_unused]] auto _ = indent();
    for (const auto& constant : constants) {
      writeConstant(constant);
    }
  }

  void InspectWriter::writeChunk(const code::Chunk& chunk) {
    os_ << fmt::format("CHUNK {}\n", chunk.name);
    [[maybe_unused]] auto _ = indent();
    os_ << formatIndented("CODE x{:X}\n", chunk.code.size());
    writeCode(chunk.code);
    os_ << formatIndented("IN x{:X}\n", chunk.inCount);
    os_ << formatIndented("OUT x{:X}\n", chunk.outCount);
  }

  void InspectWriter::writeConstant(const be::Constant& constant) {
    ConstantWriter writer;
    auto written = std::visit(writer, constant);
    os_ << formatIndented("{}\n", written);
  }

  void InspectWriter::writeCode(const code::Bytes& bytes) {
    [[maybe_unused]] auto _ = indent();
    for (auto i = bytes.begin(); i != bytes.end(); ++i) {
      switch (*i) {
        case code::Instruction::PUSH:
        case code::Instruction::MULTIPOP:
        case code::Instruction::GET_VAL:
        case code::Instruction::SET_VAL:
        case code::Instruction::CAST_IU:
        case code::Instruction::CAST_UI:
        case code::Instruction::CAST_FI:
        case code::Instruction::CAST_FU:
        case code::Instruction::CAST_WIDTH:
        case code::Instruction::RESERVE:
          emitInstructionWithArg(*i, *(i + 1));
          ++i;
          break;
        case code::Instruction::CALL:
          emitInstruction(*i++);
          emitLongArg(*i, *(i + 1), *(i + 2), *(i + 3));
          i += 3;
          break;
        default:
          emitInstruction(*i);
          break;
      }
    }
  }

  void InspectWriter::emitInstruction(uint8_t instruction) {
    os_ << formatIndented("{}\n", instructionNames[instruction]);
  }
  void InspectWriter::emitInstructionWithArg(uint8_t instruction, uint8_t arg) {
    os_ << formatIndented("{} x{:X}\n", instructionNames[instruction], arg);
  }

  void InspectWriter::emitLongArg(uint8_t arg0, uint8_t arg1, uint8_t arg2, uint8_t arg3) {
    os_ << formatIndented("x{:X} x{:X} x{:X} x{:X}\n", arg0, arg1, arg2, arg3);
  }

}  // namespace fluir
