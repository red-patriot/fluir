#include "compiler/backend/inspect_writer.hpp"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "compiler/backend/bytecode_generator.hpp"

namespace fc = fluir::code;
using namespace fc::value_literals;

TEST(TestInspectWriter, WriteEmptyProgram) {
  std::string expected = R"(I010C1100000000000000FF
CHUNK main
  CONSTANTS x0
  CODE x1
    IEXIT
  IN x0
  OUT x0
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
    VF64 100.000000000000
    VF64 3.500000000000
    VF64 4.400000000000
  CODE x13
    IPUSH x0
    IPUSH x1
    IF64_NEG
    IPUSH x2
    IF64_DIV
    IF64_ADD
    IPOP
    IPUSH x1
    IF64_NEG
    IPUSH x2
    IF64_DIV
    IF64_NEG
    IPOP
    IEXIT
  IN x0
  OUT x0
)";

  fc::ByteCode code{.header = {.filetype = '\0', .major = 24, .minor = 6, .patch = 16, .entryOffset = 5},
                    .chunks = {fc::Chunk{.name = "bar",
                                         .code =
                                           {
                                             fc::PUSH,
                                             0x00,
                                             fc::PUSH,
                                             0x01,
                                             fc::F64_NEG,
                                             fc::PUSH,
                                             0x02,
                                             fc::F64_DIV,
                                             fc::F64_ADD,
                                             fc::POP,
                                             fc::PUSH,
                                             0x01,
                                             fc::F64_NEG,
                                             fc::PUSH,
                                             0x02,
                                             fc::F64_DIV,
                                             fc::F64_NEG,
                                             fc::POP,
                                             fc::Instruction::EXIT,
                                           },
                                         .constants = {100.0_f64, 3.5_f64, 4.4_f64}}}};

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
    VF64 102.000000000000
    VF64 3.512300000000
    VF64 4.460000000000
  CODE xB
    IPUSH x0
    IPUSH x1
    IF64_NEG
    IPUSH x2
    IF64_DIV
    IF64_ADD
    IPOP
    IEXIT
  IN x0
  OUT x0
)";

  fc::ByteCode code{.header = {.filetype = '\0', .major = 4, .minor = 7, .patch = 17, .entryOffset = 15},
                    .chunks = {fc::Chunk{.name = "bar",
                                         .code =
                                           {
                                             fc::PUSH,
                                             0x00,
                                             fc::PUSH,
                                             0x01,
                                             fc::F64_NEG,
                                             fc::PUSH,
                                             0x02,
                                             fc::F64_DIV,
                                             fc::F64_ADD,
                                             fc::POP,
                                             fc::EXIT,
                                           },
                                         .constants = {102.0_f64, 3.5123_f64, 4.46_f64}}}};

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

TEST(TestInspectWriter, WriteFloatInstructions) {
  std::string expected = R"(I0120030000000000000000
CHUNK main
  CONSTANTS x0
  CODE x9
    IF64_ADD
    IF64_SUB
    IF64_MUL
    IF64_DIV
    IF64_NEG
    IF64_AFF
    IF64_INC
    IF64_DEC
    IEXIT
  IN x0
  OUT x0
)";
  fluir::code::ByteCode code{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                             .chunks = {fluir::code::Chunk{.name = "main",
                                                           .code =
                                                             {
                                                               fc::F64_ADD,
                                                               fc::F64_SUB,
                                                               fc::F64_MUL,
                                                               fc::F64_DIV,
                                                               fc::F64_NEG,
                                                               fc::F64_AFF,
                                                               fc::F64_INC,
                                                               fc::F64_DEC,
                                                               fc::EXIT,
                                                             },
                                                           .constants = {}}}};

  std::stringstream ss;
  fluir::InspectWriter uut{};
  fluir::writeCode(code, uut, ss);

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

TEST(TestInspectWriter, WriteIntInstructions) {
  std::string expected = R"(I0120030000000000000000
CHUNK main
  CONSTANTS x0
  CODE x9
    II64_ADD
    II64_SUB
    II64_MUL
    II64_DIV
    II64_NEG
    II64_AFF
    II64_INC
    II64_DEC
    IEXIT
  IN x0
  OUT x0
)";
  fluir::code::ByteCode code{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                             .chunks = {fluir::code::Chunk{.name = "main",
                                                           .code =
                                                             {
                                                               fc::I64_ADD,
                                                               fc::I64_SUB,
                                                               fc::I64_MUL,
                                                               fc::I64_DIV,
                                                               fc::I64_NEG,
                                                               fc::I64_AFF,
                                                               fc::I64_INC,
                                                               fc::I64_DEC,
                                                               fc::EXIT,
                                                             },
                                                           .constants = {}}}};

  std::stringstream ss;
  fluir::InspectWriter uut{};
  fluir::writeCode(code, uut, ss);

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

TEST(TestInspectWriter, WriteUintInstructions) {
  std::string expected = R"(I0120030000000000000000
CHUNK main
  CONSTANTS x0
  CODE x8
    IU64_ADD
    IU64_SUB
    IU64_MUL
    IU64_DIV
    IU64_AFF
    IU64_INC
    IU64_DEC
    IEXIT
  IN x0
  OUT x0
)";
  fluir::code::ByteCode code{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                             .chunks = {fluir::code::Chunk{.name = "main",
                                                           .code =
                                                             {
                                                               fc::U64_ADD,
                                                               fc::U64_SUB,
                                                               fc::U64_MUL,
                                                               fc::U64_DIV,
                                                               fc::U64_AFF,
                                                               fc::U64_INC,
                                                               fc::U64_DEC,
                                                               fc::EXIT,
                                                             },
                                                           .constants = {}}}};

  std::stringstream ss;
  fluir::InspectWriter uut{};
  fluir::writeCode(code, uut, ss);

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

TEST(TestInspectWriter, WriteIntConstants) {
  std::string expected = R"(I0120030000000000000000
CHUNK main
  CONSTANTS x4
    VI64 x123
    VI32 x345
    VI16 x542
    VI8  x1
  CODE x0
  IN x0
  OUT x0
)";
  fluir::code::ByteCode code{
    .header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
    .chunks = {fluir::code::Chunk{.name = "main",
                                  .code = {},
                                  .constants = {fluir::code::Value{static_cast<std::int64_t>(0x123)},
                                                fluir::code::Value{static_cast<std::int32_t>(0x345)},
                                                fluir::code::Value{static_cast<std::int16_t>(0x542)},
                                                fluir::code::Value{static_cast<std::int8_t>(0x1)}}}}};

  std::stringstream ss;
  fluir::InspectWriter uut{};
  fluir::writeCode(code, uut, ss);

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

TEST(TestInspectWriter, WriteUIntConstants) {
  std::string expected = R"(I0120030000000000000000
CHUNK main
  CONSTANTS x4
    VU64 x123
    VU32 x345
    VU16 x542
    VU8  x1
  CODE x0
  IN x0
  OUT x0
)";
  fluir::code::ByteCode code{
    .header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
    .chunks = {fluir::code::Chunk{.name = "main",
                                  .code = {},
                                  .constants = {fluir::code::Value{static_cast<std::uint64_t>(0x123)},
                                                fluir::code::Value{static_cast<std::uint32_t>(0x345)},
                                                fluir::code::Value{static_cast<std::uint16_t>(0x542)},
                                                fluir::code::Value{static_cast<std::uint8_t>(0x1)}}}}};

  std::stringstream ss;
  fluir::InspectWriter uut{};
  fluir::writeCode(code, uut, ss);

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}

TEST(TestInspectWriter, WriteCastInstructions) {
  std::string expected = R"(I0120030000000000000000
CHUNK main
  CONSTANTS x0
  CODE x13
    ICAST_IU x1
    ICAST_UI x2
    ICAST_IF
    ICAST_UF
    ICAST_FU x4
    ICAST_FI x8
    ICAST_WIDTH x1
    ICAST_WIDTH x2
    ICAST_WIDTH x4
    ICAST_WIDTH x8
    IEXIT
  IN x0
  OUT x0
)";
  fluir::code::ByteCode code{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                             .chunks = {fluir::code::Chunk{.name = "main",
                                                           .code = {fc::CAST_IU,
                                                                    0x1,
                                                                    fc::CAST_UI,
                                                                    0x2,
                                                                    fc::CAST_IF,
                                                                    fc::CAST_UF,
                                                                    fc::CAST_FU,
                                                                    0x4,
                                                                    fc::CAST_FI,
                                                                    0x8,
                                                                    fc::CAST_WIDTH,
                                                                    0x01,
                                                                    fc::CAST_WIDTH,
                                                                    0x02,
                                                                    fc::CAST_WIDTH,
                                                                    0x04,
                                                                    fc::CAST_WIDTH,
                                                                    0x08,
                                                                    fc::EXIT},
                                                           .constants = {}}}};

  std::stringstream ss;
  fluir::InspectWriter uut{};
  fluir::writeCode(code, uut, ss);

  auto actual = ss.str();

  EXPECT_EQ(expected, actual);
}
