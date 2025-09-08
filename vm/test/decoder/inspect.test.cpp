#include "vm/decoder/inspect.hpp"

#include <string>

#include <gtest/gtest.h>

#include "bytecode/byte_code.hpp"
#include "bytecode_assertions.hpp"

using enum fluir::code::Instruction;

TEST(TestInspectDecoder, ParsesSingleFunction) {
  std::string source = R"(I0120030000000000000000
CHUNK main
CONSTANTS x0D
VFP 0.0
VFP 1.0
VFP 2.0
VFP 3.0
VFP 4.0
VFP 5.0
VFP 6.0
VFP 7.0
VFP 8.0
VFP 9.0
VFP 10.0
VFP 11.0
VFP 12.0
CODE x0A
IPUSH_F64
x00
IPUSH_F64
x02
IF64_ADD
IPUSH_F64
x0D
IF64_MUL
IPOP
IEXIT
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
    .chunks = {fluir::code::Chunk{.name = "main",
                                  .code =
                                    {
                                      PUSH_F64,
                                      0x00,
                                      PUSH_F64,
                                      0x02,
                                      F64_ADD,
                                      PUSH_F64,
                                      0x0D,
                                      F64_MUL,
                                      POP,
                                      EXIT,
                                    },
                                  .constants = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0}}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestInspectDecoder, ParsesMultipleFunctions) {
  std::string source = R"(I07220A000000000000001A
CHUNK main
CONSTANTS x0B
VFP 0.0 VFP 1.0 VFP 2.0 VFP 3.0
VFP 4.0 VFP 5.0 VFP 6.0 VFP 7.0
VFP 8.0 VFP 9.0 VFP 10.0
CODE x07
IPUSH_F64 x00
IPUSH_F64 x02
IF64_ADD
IPOP
IEXIT
CHUNK foo
CONSTANTS x01 VFP 3.5
CODE x0A
IPUSH_F64 x00
IPUSH_F64 x00
IF64_SUB
IPUSH_F64 x00
IPOP
IPOP
IEXIT
)";
  fluir::code::ByteCode expected{
    .header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
    .chunks = {fluir::code::Chunk{.name = "main",
                                  .code =
                                    {
                                      PUSH_F64,
                                      0x00,
                                      PUSH_F64,
                                      0x02,
                                      F64_ADD,
                                      POP,
                                      EXIT,
                                    },
                                  .constants = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0}},
               fluir::code::Chunk{.name = "foo",
                                  .code =
                                    {
                                      PUSH_F64,
                                      0x00,
                                      PUSH_F64,
                                      0x00,
                                      F64_SUB,
                                      PUSH_F64,
                                      0x00,
                                      POP,
                                      POP,
                                      EXIT,
                                    },
                                  .constants = {3.5}}}};

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
    VFP 7.654300000000
    VFP 1.234500000000
    VFP 6.789000000000
  CODE xF
    IPUSH_F64 x0
    IPUSH_F64 x1
    IF64_AFF
    IF64_MUL
    IPOP
    IPUSH_F64 x1
    IPUSH_F64 x2
    IF64_DIV
    IF64_NEG
    IPOP
    IEXIT
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 7, .minor = 34, .patch = 10, .entryOffset = 26},
                                 .chunks = {fluir::code::Chunk{.name = "foo",
                                                               .code = {PUSH_F64,
                                                                        0x00,
                                                                        PUSH_F64,
                                                                        0x01,
                                                                        F64_AFF,
                                                                        F64_MUL,
                                                                        POP,
                                                                        PUSH_F64,
                                                                        0x01,
                                                                        PUSH_F64,
                                                                        0x02,
                                                                        F64_DIV,
                                                                        F64_NEG,
                                                                        POP,
                                                                        EXIT},
                                                               .constants = {
                                                                 7.654300000000,
                                                                 1.234500000000,
                                                                 6.789000000000,
                                                               }}}};

  auto actual = fluir::InspectDecoder{}.decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_EQ(expected.chunks.size(), actual.chunks.size());
  for (int i = 0; i != expected.chunks.size(); ++i) {
    EXPECT_CHUNK_EQ(expected.chunks.at(i), actual.chunks.at(i));
  }
}
