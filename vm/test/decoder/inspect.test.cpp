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
IPUSH_FP
x00
IPUSH_FP
x02
IFP_ADD
IPUSH_FP
x0D
IFP_MULTIPLY
IPOP
IEXIT
)";
  fluir::code::ByteCode expected{
      .header = {.filetype = 'I',
                 .major = 1,
                 .minor = 32,
                 .patch = 3,
                 .entryOffset = 0},
      .chunks = {
          fluir::code::Chunk{
              .name = "main",
              .code = {
                  PUSH_FP,
                  0x00,
                  PUSH_FP,
                  0x02,
                  FP_ADD,
                  PUSH_FP,
                  0x0D,
                  FP_MULTIPLY,
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
IPUSH_FP x00
IPUSH_FP x02
IFP_ADD
IPOP
IEXIT
CHUNK foo
CONSTANTS x01 VFP 3.5
CODE x0A
IPUSH_FP x00
IPUSH_FP x00
IFP_SUBTRACT
IPUSH_FP x00
IPOP
IPOP
IEXIT
)";
  fluir::code::ByteCode expected{
      .header = {.filetype = 'I',
                 .major = 7,
                 .minor = 34,
                 .patch = 10,
                 .entryOffset = 26},
      .chunks = {
          fluir::code::Chunk{.name = "main", .code = {
                                                 PUSH_FP,
                                                 0x00,
                                                 PUSH_FP,
                                                 0x02,
                                                 FP_ADD,
                                                 POP,
                                                 EXIT,
                                             },
                             .constants = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0}},
          fluir::code::Chunk{.name = "foo", .code = {
                                                PUSH_FP,
                                                0x00,
                                                PUSH_FP,
                                                0x00,
                                                FP_SUBTRACT,
                                                PUSH_FP,
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
