#include "vm/decoder/decode.hpp"

#include <gtest/gtest.h>

#include "bytecode_assertions.hpp"

TEST(TestDecodeInspect, DecodesHeaderInformation) {
  fluir::code::Header expected{.filetype = 'I',
                               .major = 12,
                               .minor = 1,
                               .patch = 2,
                               .entryOffset = 12345678};

  std::string data = R"(I0C01020000000000BC614E
)";

  auto code = fluir::decodeInspectCode(data);
  auto& actual = code.header;

  EXPECT_BC_HEADER_EQ(expected, actual);
}
