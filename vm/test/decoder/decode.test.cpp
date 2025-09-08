#include "vm/decoder/decode.hpp"

#include <string>

#include <gtest/gtest.h>

#include "bytecode/byte_code.hpp"
#include "bytecode_assertions.hpp"

using enum fluir::code::Instruction;

TEST(TestDecoder, SelectsInspectAndDecodesCorrectly) {
  std::string source = R"(I0120030000000000000000
CHUNK main
CONSTANTS x0D
VFP 0.0 VFP 1.0 VFP 2.0 VFP 3.0
VFP 4.0 VFP 5.0 VFP 6.0 VFP 7.0
VFP 8.0 VFP 9.0 VFP 10.0 VFP 11.0
VFP 12.0
CODE x0A
IPUSH_F64 x00 IPUSH_F64 x02
IF64_ADD IPUSH_F64 x0D IF64_DIV
IPOP IEXIT
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
                                      F64_DIV,
                                      POP,
                                      EXIT,
                                    },
                                  .constants = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0}}}};

  auto actual = fluir::decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestDecoder, DetectsInvalidFileType) {
  std::string source = R"(C0120030000000000000000
CHUNK main
CONSTANTS x0D
VFP 0.0 VFP 1.0 VFP 2.0 VFP 3.0
VFP 4.0 VFP 5.0 VFP 6.0 VFP 7.0
VFP 8.0 VFP 9.0 VFP 10.0 VFP 11.0
VFP 12.0
CODE x0A
IPUSH_F64 x00 IPUSH_F64 x02
IF64_ADD IPUSH_F64 x0D IF64_DIV
IPOP IEXIT
)";

  EXPECT_THROW(fluir::decode(source), std::runtime_error);
}
