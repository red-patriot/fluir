#include "vm/decoder/inspect.hpp"

#include <string>

#include <gtest/gtest.h>

#include "bytecode_assertions.hpp"
#include "vm/code/byte_code.hpp"

using enum fluir::code::Instruction;
using enum fluir::code::NumericWidth;
using namespace fluir::code::value_literals;

TEST(TestInspectDecoder, ParsesSingleFunction) {
  std::string source = R"(I0120030000000000000000
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
CHUNK main
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
IN x0
OUT x0
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
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
                                               12.0_f64},
                                 .chunks = {fluir::code::Chunk{
                                   .name = "main",
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
                                 }}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesFloatInstructions) {
  std::string source = R"(I0120030000000000000000
CONSTANTS x00
CHUNK main
CODE x08
IF64_ADD
IF64_SUB
IF64_MUL
IF64_DIV
IF64_INC
IF64_DEC
IF64_NEG
IEXIT
IN x0
OUT x0
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                                 .constants = {},
                                 .chunks = {fluir::code::Chunk{.name = "main",
                                                               .code = {
                                                                 F64_ADD,
                                                                 F64_SUB,
                                                                 F64_MUL,
                                                                 F64_DIV,
                                                                 F64_INC,
                                                                 F64_DEC,
                                                                 F64_NEG,
                                                                 EXIT,
                                                               }}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesIntInstructions) {
  std::string source = R"(I0120030000000000000000
CONSTANTS x00
CHUNK main
CODE x08
II64_ADD
II64_SUB
II64_MUL
II64_DIV
II64_INC
II64_DEC
II64_NEG
IEXIT
IN x0
OUT x0
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                                 .constants = {},
                                 .chunks = {fluir::code::Chunk{.name = "main",
                                                               .code = {
                                                                 I64_ADD,
                                                                 I64_SUB,
                                                                 I64_MUL,
                                                                 I64_DIV,
                                                                 I64_INC,
                                                                 I64_DEC,
                                                                 I64_NEG,
                                                                 EXIT,
                                                               }}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesUintInstructions) {
  std::string source = R"(I0120030000000000000000
CONSTANTS x00
CHUNK main
CODE x07
IU64_ADD
IU64_SUB
IU64_MUL
IU64_DIV
IU64_INC
IU64_DEC
IEXIT
IN x0
OUT x0
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                                 .constants = {},
                                 .chunks = {fluir::code::Chunk{.name = "main",
                                                               .code = {
                                                                 U64_ADD,
                                                                 U64_SUB,
                                                                 U64_MUL,
                                                                 U64_DIV,
                                                                 U64_INC,
                                                                 U64_DEC,
                                                                 EXIT,
                                                               }}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesIntConstants) {
  std::string source = R"(I0120030000000000000000
CHUNK main
CODE x00
IN x0
OUT x0
CONSTANTS x04
VI64 x123
VI32 x345
VI16 x542
VI8  x1
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                                 .constants = {fluir::code::Value{static_cast<std::int64_t>(0x123)},
                                               fluir::code::Value{static_cast<std::int32_t>(0x345)},
                                               fluir::code::Value{static_cast<std::int16_t>(0x542)},
                                               fluir::code::Value{static_cast<std::int8_t>(0x1)}},
                                 .chunks = {fluir::code::Chunk{
                                   .name = "main",
                                   .code = {},
                                 }}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesUIntConstants) {
  std::string source = R"(I0120030000000000000000
CONSTANTS x04
VU64 x123
VU32 x345
VU16 x542
VU8  x1
CHUNK main
CODE x00
IN x0
OUT x0
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
                                 .constants = {fluir::code::Value{static_cast<std::uint64_t>(0x123)},
                                               fluir::code::Value{static_cast<std::uint32_t>(0x345)},
                                               fluir::code::Value{static_cast<std::uint16_t>(0x542)},
                                               fluir::code::Value{static_cast<std::uint8_t>(0x1)}},
                                 .chunks = {fluir::code::Chunk{.name = "main", .code = {}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesMultipleFunctions) {
  std::string source = R"(I07220A000000000000001A
CONSTANTS x0C
VF64 0.0 VF64 1.0 VF64 2.0 VF64 3.0
VF64 4.0 VF64 5.0 VF64 6.0 VF64 7.0
VF64 8.0 VF64 9.0 VF64 10.0 VF64 3.5
CHUNK main
CODE x07
IPUSH x00
IPUSH x02
IF64_ADD
IPOP
IEXIT
IN x0
OUT x0
CHUNK foo
CODE x0A
IPUSH x00
IPUSH x00
IF64_SUB
IPUSH x00
IPOP
IPOP
IEXIT
IN x0
OUT x0
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
    .constants =
      {0.0_f64, 1.0_f64, 2.0_f64, 3.0_f64, 4.0_f64, 5.0_f64, 6.0_f64, 7.0_f64, 8.0_f64, 9.0_f64, 10.0_f64, 3.5_f64},
    .chunks = {fluir::code::Chunk{
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
               },
               fluir::code::Chunk{.name = "foo",
                                  .code = {
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
                                  }}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_EQ(expected.chunks.size(), actual.chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.chunks.at(i));
  }
}

TEST(TestInspectDecoder, DecodesUnaryInstructionsCorrectly) {
  std::string source = R"(I07220A000000000000001A
CONSTANTS x3
  VF64 7.654300000000
  VF64 1.234500000000
  VF64 6.789000000000
CHUNK foo
  CODE xE
    IPUSH x0
    IPUSH x1
    IF64_MUL
    IPOP
    IPUSH x1
    IPUSH x2
    IF64_DIV
    IF64_NEG
    IPOP
    IEXIT
  IN x0
  OUT x0
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
    .constants =
      {
        7.654300000000_f64,
        1.234500000000_f64,
        6.789000000000_f64,
      },
    .chunks = {fluir::code::Chunk{
      .name = "foo",
      .code = {PUSH, 0x00, PUSH, 0x01, F64_MUL, POP, PUSH, 0x01, PUSH, 0x02, F64_DIV, F64_NEG, POP, EXIT}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_EQ(expected.chunks.size(), actual.chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.chunks.at(i));
  }
}

TEST(TestInspectDecoder, DecodesCastingInstructionsCorrectly) {
  std::string source = R"(I07220A000000000000001A
CONSTANTS x3
  VF64 7.654300000000
  VF64 1.234500000000
  VF64 6.789000000000
CHUNK foo
  CODE xF
    ICAST_IU
    ICAST_UI
    ICAST_IF
    ICAST_UF
    ICAST_FI
    ICAST_FU
    ICAST_WIDTH x1
    ICAST_WIDTH x2
    ICAST_WIDTH x4
    ICAST_WIDTH x8
    IEXIT
  IN x0
  OUT x0
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
                                 .constants =
                                   {
                                     7.654300000000_f64,
                                     1.234500000000_f64,
                                     6.789000000000_f64,
                                   },
                                 .chunks = {fluir::code::Chunk{.name = "foo",
                                                               .code = {CAST_IU,
                                                                        CAST_UI,
                                                                        CAST_IF,
                                                                        CAST_UF,
                                                                        CAST_FI,
                                                                        CAST_FU,
                                                                        CAST_WIDTH,
                                                                        WIDTH_8,
                                                                        CAST_WIDTH,
                                                                        WIDTH_16,
                                                                        CAST_WIDTH,
                                                                        WIDTH_32,
                                                                        CAST_WIDTH,
                                                                        WIDTH_64,
                                                                        EXIT}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_EQ(expected.chunks.size(), actual.chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.chunks.at(i));
  }
}

TEST(TestInspectDecoder, DecodesGetSetValInstructions) {
  std::string source = R"(I07220A000000000000001A
CONSTANTS x1
  VF64 7.0
CHUNK foo
  CODE x9
    IPUSH x0
    IGET_VAL x0
    ISET_VAL x0
    IPOP
    IPOP
    IEXIT
  IN x0
  OUT x0
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
    .constants = {7.000000000000_f64},
    .chunks = {fluir::code::Chunk{.name = "foo", .code = {PUSH, 0x0, GET_VAL, 0x0, SET_VAL, 0x0, POP, POP, EXIT}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_EQ(expected.chunks.size(), actual.chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.chunks.at(i));
  }
}

TEST(TestInspectDecoder, DecodesMultipopInstruction) {
  std::string source = R"(I07220A000000000000001A
CONSTANTS x1
    VF64 7.0
CHUNK pops
  CODE x7
    IPUSH x0
    IPUSH x0
    IMULTIPOP x2
    IEXIT
  IN x0
  OUT x0
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
    .constants = {7.000000000000_f64},
    .chunks = {fluir::code::Chunk{.name = "pops", .code = {PUSH, 0x0, PUSH, 0x0, MULTIPOP, 0x2, EXIT}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_EQ(expected.chunks.size(), actual.chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.chunks.at(i));
  }
}

TEST(TestInspectDecoder, DecodesInOutCountCorrectly) {
  std::string source = R"(I07220A000000000000001A
CONSTANTS x1
  VF64 7.0
CHUNK junk
  CODE x0
  IN x2
  OUT x1
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
    .constants = {7.000000000000_f64},
    .chunks = {fluir::code::Chunk{.name = "junk", .code = {}, .inCount = 2, .outCount = 1}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_EQ(expected.chunks.size(), actual.chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.chunks.at(i));
  }
}

TEST(TestInspectDecoder, ParsesStringConstant) {
  std::string source = R"(I0120030000000000000000
CONSTANTS x03
VSTR s00000000
VSTR s00000004okay
VSTR s0000001Aabcdefghijklmnopqrstuvwxyz
CHUNK main
CODE x00
IN x0
OUT x0
)";
  const fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
    .constants = {fluir::code::Value{fluir::createStaticString("")},
                  fluir::code::Value{fluir::createStaticString("okay")},
                  fluir::code::Value{fluir::createStaticString("abcdefghijklmnopqrstuvwxyz")}},
    .chunks = {fluir::code::Chunk{.name = "main", .code = {}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_BC_VALUES_EQ(expected.constants, actual.constants);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesBoolConstant) {
  std::string source = R"(I0120030000000000000000
CONSTANTS x02
VTRUE
VFALSE
CHUNK main
CODE x00
IN x0
OUT x0
)";
  const fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
    .constants = {fluir::code::Value{true}, fluir::code::Value{false}},
    .chunks = {fluir::code::Chunk{.name = "main", .code = {}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_BC_VALUES_EQ(expected.constants, actual.constants);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}
