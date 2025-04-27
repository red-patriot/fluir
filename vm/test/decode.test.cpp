#include "vm/decode.hpp"

#include <gtest/gtest.h>

TEST(TestDecodeInspect, DecodesHeaderInformation) {
  fluir::code::Header expected{.filetype = 'I',
                               .major = 12,
                               .minor = 1,
                               .patch = 2,
                               .entryOffset = 12345678};

  std::string data = R"(I01200100200000000000012345678
)";

  auto code = fluir::decodeInspectCode(data);
  auto& actual = code.header;

  EXPECT_EQ(expected.filetype, actual.filetype);
  EXPECT_EQ(expected.major, actual.major);
  EXPECT_EQ(expected.minor, actual.minor);
  EXPECT_EQ(expected.patch, actual.patch);
  EXPECT_EQ(expected.entryOffset, actual.entryOffset);
}
