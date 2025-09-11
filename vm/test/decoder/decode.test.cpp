#include "vm/decoder/decode.hpp"

#include <string>

#include <gtest/gtest.h>

#include "bytecode/byte_code.hpp"
#include "bytecode_assertions.hpp"

using enum fluir::code::Instruction;
using namespace fluir::code::value_literals;

TEST(TestDecoder, SelectsInspectAndDecodesCorrectly) {
  std::string source = R"(I0120030000000000000000
CHUNK main
CONSTANTS x0D
VF64 0.0 VF64 1.0 VF64 2.0 VF64 3.0
VF64 4.0 VF64 5.0 VF64 6.0 VF64 7.0
VF64 8.0 VF64 9.0 VF64 10.0 VF64 11.0
VF64 12.0
CODE x0A
IPUSH_F64 x00 IPUSH_F64 x02
IF64_ADD IPUSH_F64 x0D IF64_DIV
IPOP IEXIT
)";
  fluir::code::ByteCode expected{.header = {.filetype = 'I', .major = 1, .minor = 32, .patch = 3, .entryOffset = 0},
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

  auto actual = fluir::decode(source);

  EXPECT_BC_HEADER_EQ(expected.header, actual.header);
  EXPECT_CHUNK_EQ(expected.chunks.at(0), actual.chunks.at(0));
}

TEST(TestDecoder, DetectsInvalidFileType) {
  std::string source = R"(C0120030000000000000000
CHUNK main
CONSTANTS x0D
VF64 0.0 VF64 1.0 VF64 2.0 VF64 3.0
VF64 4.0 VF64 5.0 VF64 6.0 VF64 7.0
VF64 8.0 VF64 9.0 VF64 10.0 VF64 11.0
VF64 12.0
CODE x0A
IPUSH_F64 x00 IPUSH_F64 x02
IF64_ADD IPUSH_F64 x0D IF64_DIV
IPOP IEXIT
)";

  EXPECT_THROW(fluir::decode(source), std::runtime_error);
}
