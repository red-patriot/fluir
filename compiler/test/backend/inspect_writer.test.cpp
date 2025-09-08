#include "compiler/backend/inspect_writer.hpp"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "compiler/backend/bytecode_generator.hpp"

namespace fc = fluir::code;

TEST(TestInspectWriter, WriteEmptyProgram) {
  std::string expected = R"(I010C1100000000000000FF
CHUNK main
  CONSTANTS x0
  CODE x1
    IEXIT
)";

  fc::ByteCode code{.header = {.filetype = '\0', .major = 1, .minor = 12, .patch = 17, .entryOffset = 255},
                    .chunks = {fc::Chunk{.name = "main", .code = {fc::Instruction::EXIT}, .constants = {}}}};

  std::stringstream ss;
  fluir::InspectWriter uut{};
  fluir::writeCode(code, uut, ss);

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

TEST(TestInspectWriter, WriteSimpleProgram) {
  std::string expected = R"(I1806100000000000000005
CHUNK bar
  CONSTANTS x3
    VFP 100.000000000000
    VFP 3.500000000000
    VFP -4.400000000000
  CODE x13
    IPUSH_FP x0
    IPUSH_FP x1
    IF64_NEG
    IPUSH_FP x2
    IF64_DIV
    IF64_ADD
    IPOP
    IPUSH_FP x1
    IF64_NEG
    IPUSH_FP x2
    IF64_DIV
    IF64_NEG
    IPOP
    IEXIT
)";

  fc::ByteCode code{
    .header = {.filetype = '\0', .major = 24, .minor = 6, .patch = 16, .entryOffset = 5},
    .chunks = {
      fc::Chunk{.name = "bar",
                .code =
                  {
                    fc::PUSH_FP, 0x00,    fc::PUSH_FP,           0x01, fc::F64_NEG, fc::PUSH_FP, 0x02, fc::F64_DIV,
                    fc::F64_ADD, fc::POP, fc::PUSH_FP,           0x01, fc::F64_NEG, fc::PUSH_FP, 0x02, fc::F64_DIV,
                    fc::F64_NEG, fc::POP, fc::Instruction::EXIT,
                  },
                .constants = {100.0, 3.5, -4.4}}}};

  std::stringstream ss;
  fluir::InspectWriter uut{};
  fluir::writeCode(code, uut, ss);

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

TEST(TestInspectWriter, WriteProgramToFile) {
  std::string expected = R"(I040711000000000000000F
CHUNK bar
  CONSTANTS x3
    VFP 102.000000000000
    VFP 3.512300000000
    VFP 4.460000000000
  CODE xB
    IPUSH_FP x0
    IPUSH_FP x1
    IF64_NEG
    IPUSH_FP x2
    IF64_DIV
    IF64_ADD
    IPOP
    IEXIT
)";

  fc::ByteCode code{.header = {.filetype = '\0', .major = 4, .minor = 7, .patch = 17, .entryOffset = 15},
                    .chunks = {fc::Chunk{.name = "bar",
                                         .code =
                                           {
                                             fc::PUSH_FP,
                                             0x00,
                                             fc::PUSH_FP,
                                             0x01,
                                             fc::F64_NEG,
                                             fc::PUSH_FP,
                                             0x02,
                                             fc::F64_DIV,
                                             fc::F64_ADD,
                                             fc::POP,
                                             fc::EXIT,
                                           },
                                         .constants = {102.0, 3.5123, 4.46}}}};

  auto file = std::filesystem::temp_directory_path() / "code.flc";
  {
    std::ofstream fout{file};
    fluir::InspectWriter uut{};
    fluir::writeCode(code, uut, fout);
  }

  std::stringstream ss;
  {
    std::ifstream fin{file};
    ss << fin.rdbuf();
  }

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}
