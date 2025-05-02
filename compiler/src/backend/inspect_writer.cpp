#include "compiler/backend/inspect_writer.hpp"

#include <string>

#include "fmt/format.h"

namespace fluir {
  namespace {
#define STRINGIFY(i) #i
#define FLUIR_INSTRUCTION_TO_STR(inst) STRINGIFY(I##inst),
    std::string instructionNames[] = {
        FLUIR_CODE_INSTRUCTIONS(FLUIR_INSTRUCTION_TO_STR)};
#undef FLUIR_INSTRUCTION_TO_STR
#undef STRINGIFY
  }  // namespace

  template <typename... Args>
  std::string InspectWriter::formatIndented(fmt::format_string<Args...> format, Args&&... args) {
    return fmt::format("{}", indentation()) + fmt::format(format, std::forward<Args>(args)...);
  }

  void InspectWriter::writeHeader(const code::Header& header,
                                  std::ostream& os) {
    os << fmt::format("I{:0>2X}{:0>2X}{:0>2X}{:0>16X}\n", header.major, header.minor, header.patch,
                      header.entryOffset);
  }
  void InspectWriter::writeChunk(const code::Chunk& chunk,
                                 std::ostream& os) {
    os << fmt::format("CHUNK {}\n", chunk.name);
    indent();
    os << formatIndented("CONSTANTS x{:X}\n", chunk.constants.size());
    writeConstants(chunk.constants, os);

    os << formatIndented("CODE x{:X}\n",
                         chunk.code.size());

    writeCode(chunk.code, os);

    dedent();
  }

  void InspectWriter::writeConstants(const std::vector<code::Value>& constants, std::ostream& os) {
    indent();
    for (const auto& constant : constants) {
      writeConstant(constant, os);
    }
    dedent();
  }

  void InspectWriter::writeConstant(const code::Value& constant, std::ostream& os) {
    os << formatIndented("VFP {:.12f}\n", constant);
  }

  void InspectWriter::writeCode(const code::Bytes& bytes, std::ostream& os) {
    indent();
    for (auto i = bytes.begin(); i != bytes.end(); ++i) {
      switch (*i) {
        case code::Instruction::PUSH_FP:
          os << formatIndented("{} x{:X}\n",
                               instructionNames[*i],
                               *(i + 1));
          ++i;
          break;
        default:
          os << formatIndented("{}\n",
                               instructionNames[*i]);
      }
    }
    dedent();
  }

  std::string_view InspectWriter::indentation() {
    return std::string_view{indent_.c_str(), level_};
  }

  void InspectWriter::indent() {
    level_ += 2;
    if (indent_.size() < level_) {
      indent_ = std::string(level_, ' ');
    }
  }

  void InspectWriter::dedent() {
    level_ -= 2;
  }
}  // namespace fluir
