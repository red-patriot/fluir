module;

#include <string>
#include <vector>

#include <fmt/format.h>

#include "byte_code/instruction.hpp"

module fluir.backend.inspect_writer;

namespace fluir {
  namespace {
#define STRINGIFY(i) #i
#define FLUIR_INSTRUCTION_TO_STR(inst) STRINGIFY(I##inst),
    std::string instructionNames[] = {FLUIR_CODE_INSTRUCTIONS(FLUIR_INSTRUCTION_TO_STR)};
#undef FLUIR_INSTRUCTION_TO_STR
#undef STRINGIFY
  }  // namespace

  void InspectWriter::writeHeader(const code::Header& header, std::ostream& os) {
    os << fmt::format("I{:0>2X}{:0>2X}{:0>2X}{:0>16X}\n", header.major, header.minor, header.patch, header.entryOffset);
  }
  void InspectWriter::writeChunk(const code::Chunk& chunk, std::ostream& os) {
    os << fmt::format("CHUNK {}\n", chunk.name);
    [[maybe_unused]] auto _ = indent();
    os << formatIndented("CONSTANTS x{:X}\n", chunk.constants.size());
    writeConstants(chunk.constants, os);

    os << formatIndented("CODE x{:X}\n", chunk.code.size());

    writeCode(chunk.code, os);
  }

  void InspectWriter::writeConstants(const std::vector<code::Value>& constants, std::ostream& os) {
    [[maybe_unused]] auto _ = indent();
    for (const auto& constant : constants) {
      writeConstant(constant, os);
    }
  }

  void InspectWriter::writeConstant(const code::Value& constant, std::ostream& os) {
    os << formatIndented("VFP {:.12f}\n", constant);
  }

  void InspectWriter::writeCode(const code::Bytes& bytes, std::ostream& os) {
    [[maybe_unused]] auto _ = indent();
    for (auto i = bytes.begin(); i != bytes.end(); ++i) {
      switch (*i) {
        case code::Instruction::PUSH_FP:
          os << formatIndented("{} x{:X}\n", instructionNames[*i], *(i + 1));
          ++i;
          break;
        default:
          os << formatIndented("{}\n", instructionNames[*i]);
      }
    }
  }
}  // namespace fluir
