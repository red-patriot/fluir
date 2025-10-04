#include "vm/decoder/inspect.hpp"

#include <string>

#include <gtest/gtest.h>

#include "bytecode/byte_code.hpp"
#include "bytecode_assertions.hpp"

using enum fluir::code::Instruction;
using namespace fluir::code::value_literals;

TEST(TestInspectDecoder, ParsesSingleFunction) {
  std::string source = R"(I0120030000000000000000
CHUNK main
CONSTANTS x0D
VF64 0.0
VF64 1.0
VF64 2.0
VF64 3.0
VF64 4.0
VF64 5.0
VF64 6.0
VF64 7.0
VF64 8.0
VF64 9.0
VF64 10.0
VF64 11.0
VF64 12.0
CODE x0A
IPUSH
x00
IPUSH
x02
IF64_ADD
IPUSH
x0D
IF64_MUL
IPOP
IEXIT
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                                 .chunks = {fluir::code::Chunk{.name = "main",
                                                               .code =
                                                                 {
                                                                   PUSH,
                                                                   0x00,
                                                                   PUSH,
                                                                   0x02,
                                                                   F64_ADD,
                                                                   PUSH,
                                                                   0x0D,
                                                                   F64_MUL,
                                                                   POP,
                                                                   EXIT,
                                                                 },
                                                               .constants = {0.0_f64,
                                                                             1.0_f64,
                                                                             2.0_f64,
                                                                             3.0_f64,
                                                                             4.0_f64,
                                                                             5.0_f64,
                                                                             6.0_f64,
                                                                             7.0_f64,
                                                                             8.0_f64,
                                                                             9.0_f64,
                                                                             10.0_f64,
                                                                             11.0_f64,
                                                                             12.0_f64}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesIntInstructions) {
  std::string source = R"(I0120030000000000000000
CHUNK main
CONSTANTS x00
CODE x07
II64_ADD
II64_SUB
II64_MUL
II64_DIV
II64_NEG
II64_AFF
IEXIT
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                                 .chunks = {fluir::code::Chunk{.name = "main",
                                                               .code =
                                                                 {
                                                                   I64_ADD,
                                                                   I64_SUB,
                                                                   I64_MUL,
                                                                   I64_DIV,
                                                                   I64_NEG,
                                                                   I64_AFF,
                                                                   EXIT,
                                                                 },
                                                               .constants = {}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesUintInstructions) {
  std::string source = R"(I0120030000000000000000
CHUNK main
CONSTANTS x00
CODE x07
IU64_ADD
IU64_SUB
IU64_MUL
IU64_DIV
IU64_NEG
IU64_AFF
IEXIT
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                                 .chunks = {fluir::code::Chunk{.name = "main",
                                                               .code =
                                                                 {
                                                                   U64_ADD,
                                                                   U64_SUB,
                                                                   U64_MUL,
                                                                   U64_DIV,
                                                                   U64_NEG,
                                                                   U64_AFF,
                                                                   EXIT,
                                                                 },
                                                               .constants = {}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesIntConstants) {
  std::string source = R"(I0120030000000000000000
CHUNK main
CONSTANTS x04
VI64 x123
VI32 x345
VI16 x542
VI8  x1
CODE x00
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
    .chunks = {fluir::code::Chunk{.name = "main",
                                  .code = {},
                                  .constants = {fluir::code::Value{static_cast<std::int64_t>(0x123)},
                                                fluir::code::Value{static_cast<std::int32_t>(0x345)},
                                                fluir::code::Value{static_cast<std::int16_t>(0x542)},
                                                fluir::code::Value{static_cast<std::int8_t>(0x1)}}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesUIntConstants) {
  std::string source = R"(I0120030000000000000000
CHUNK main
CONSTANTS x04
VU64 x123
VU32 x345
VU16 x542
VU8  x1
CODE x00
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
    .chunks = {fluir::code::Chunk{.name = "main",
                                  .code = {},
                                  .constants = {fluir::code::Value{static_cast<std::uint64_t>(0x123)},
                                                fluir::code::Value{static_cast<std::uint32_t>(0x345)},
                                                fluir::code::Value{static_cast<std::uint16_t>(0x542)},
                                                fluir::code::Value{static_cast<std::uint8_t>(0x1)}}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesMultipleFunctions) {
  std::string source = R"(I07220A000000000000001A
CHUNK main
CONSTANTS x0B
VF64 0.0 VF64 1.0 VF64 2.0 VF64 3.0
VF64 4.0 VF64 5.0 VF64 6.0 VF64 7.0
VF64 8.0 VF64 9.0 VF64 10.0
CODE x07
IPUSH x00
IPUSH x02
IF64_ADD
IPOP
IEXIT
CHUNK foo
CONSTANTS x01 VF64 3.5
CODE x0A
IPUSH x00
IPUSH x00
IF64_SUB
IPUSH x00
IPOP
IPOP
IEXIT
)";
  fluir::code::ByteCode
    expected{
      .header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
      .chunks =
        {fluir::code::Chunk{
           .name = "main",
           .code =
             {
               PUSH,
               0x00,
               PUSH,
               0x02,
               F64_ADD,
               POP,
               EXIT,
             },
           .constants =
             {0.0_f64, 1.0_f64, 2.0_f64, 3.0_f64, 4.0_f64, 5.0_f64, 6.0_f64, 7.0_f64, 8.0_f64, 9.0_f64, 10.0_f64}},
         fluir::code::Chunk{.name = "foo",
                            .code =
                              {
                                PUSH,
                                0x00,
                                PUSH,
                                0x00,
                                F64_SUB,
                                PUSH,
                                0x00,
                                POP,
                                POP,
                                EXIT,
                              },
                            .constants = {3.5_f64}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_EQ(expected.chunks.size(), actual.chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.chunks.at(i));
  }
}

TEST(TestInspectDecoder, DecodesUnaryInstructionsCorrectly) {
  std::string source = R"(I07220A000000000000001A
CHUNK foo
  CONSTANTS x3
    VF64 7.654300000000
    VF64 1.234500000000
    VF64 6.789000000000
  CODE xF
    IPUSH x0
    IPUSH x1
    IF64_AFF
    IF64_MUL
    IPOP
    IPUSH x1
    IPUSH x2
    IF64_DIV
    IF64_NEG
    IPOP
    IEXIT
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
    .chunks = {fluir::code::Chunk{
      .name = "foo",
      .code = {PUSH, 0x00, PUSH, 0x01, F64_AFF, F64_MUL, POP, PUSH, 0x01, PUSH, 0x02, F64_DIV, F64_NEG, POP, EXIT},
      .constants = {
        7.654300000000_f64,
        1.234500000000_f64,
        6.789000000000_f64,
      }}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_EQ(expected.chunks.size(), actual.chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.chunks.at(i));
  }
}
