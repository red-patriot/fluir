#include "vm/vm.hpp"

#include <gtest/gtest.h>

namespace fc = fluir::code;
using enum fluir::code::OpCode;

TEST(TestVM, ExecEmptyFunction) {
  fluir::code::ByteCode code{.header = {},
                             .chunks = {
                                 {

                                 }}};
  code.chunks.push_back({});
  auto& mainChunk = code.chunks.at(0);
  mainChunk.code.push_back(EXIT);

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
}
