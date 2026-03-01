#include <gtest/gtest.h>

#include "vm/machine/vm.hpp"

namespace fc = fluir::code;
using enum fluir::code::Instruction;
using namespace fluir::code::value_literals;

TEST(TestVMFunctionCalls, HandlesCallWithoutParamsWithReturn) {
  std::int32_t expected = 4;

  fluir::code::ByteCode code{
    .header = {.entryOffset = 0},
    .chunks = {
      fc::Chunk{.name = "main", .code = {RESERVE, 1, CALL, 0, 0, 0, 1, EXIT}, .constants = {}},
      fc::Chunk{.name = "getVal", .code = {PUSH, 0, SET_VAL, 0, POP, RETURN}, .constants = {4_i32}, .inOutCount = 1}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().front().asI32());
}
