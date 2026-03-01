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

TEST(TestVMFunctionCalls, HandlesCallWithParamsWithReturn) {
  fluir::code::ByteCode code{.header = {.entryOffset = 0},
                             .chunks = {fc::Chunk{.name = "main",
                                                  .code = {RESERVE, 1, PUSH, 0, PUSH, 1, CALL, 0, 0, 0, 1, EXIT},
                                                  .constants = {3_i32, 7_i32}},
                                        fc::Chunk{.name = "add",
                                                  .code = {GET_VAL, 1, GET_VAL, 2, I64_ADD, SET_VAL, 0, POP, RETURN},
                                                  .constants = {},
                                                  .inOutCount = 3}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(10, uut.viewStack().front().asI32());
}

TEST(TestVMFunctionCalls, HandlesCallWithoutParamsWithoutReturn) {
  fluir::code::ByteCode code{.header = {.entryOffset = 0},
                             .chunks = {fc::Chunk{.name = "main", .code = {CALL, 0, 0, 0, 1, EXIT}, .constants = {}},
                                        fc::Chunk{.name = "noop", .code = {RETURN}, .constants = {}, .inOutCount = 0}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_TRUE(uut.viewStack().empty());
}

TEST(TestVMFunctionCalls, HandlesCallWithParamsWithoutReturn) {
  fluir::code::ByteCode code{
    .header = {.entryOffset = 0},
    .chunks = {
      fc::Chunk{.name = "main", .code = {PUSH, 0, PUSH, 1, CALL, 0, 0, 0, 1, EXIT}, .constants = {3_i32, 7_i32}},
      fc::Chunk{.name = "sink", .code = {RETURN}, .constants = {}, .inOutCount = 2}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(2u, uut.viewStack().size());
}

TEST(TestVMFunctionCalls, HandlesDeepCallStack) {
  fluir::code::ByteCode code{
    .header = {.entryOffset = 0},
    .chunks = {
      fc::Chunk{.name = "main", .code = {RESERVE, 1, PUSH, 0, CALL, 0, 0, 0, 1, EXIT}, .constants = {0_i32}},
      fc::Chunk{
        .name = "f1",
        .code =
          {GET_VAL, 1, PUSH, 0, I64_ADD, RESERVE, 1, GET_VAL, 0, CALL, 0, 0, 0, 2, GET_VAL, 1, SET_VAL, 0, RETURN},
        .constants = {10_i32},
        .inOutCount = 2},
      fc::Chunk{
        .name = "f2",
        .code =
          {GET_VAL, 1, PUSH, 0, I64_ADD, RESERVE, 1, GET_VAL, 0, CALL, 0, 0, 0, 3, GET_VAL, 1, SET_VAL, 0, RETURN},
        .constants = {10_i32},
        .inOutCount = 2},
      fc::Chunk{
        .name = "f3",
        .code =
          {GET_VAL, 1, PUSH, 0, I64_ADD, RESERVE, 1, GET_VAL, 0, CALL, 0, 0, 0, 4, GET_VAL, 1, SET_VAL, 0, RETURN},
        .constants = {10_i32},
        .inOutCount = 2},
      fc::Chunk{
        .name = "f4",
        .code =
          {GET_VAL, 1, PUSH, 0, I64_ADD, RESERVE, 1, GET_VAL, 0, CALL, 0, 0, 0, 5, GET_VAL, 1, SET_VAL, 0, RETURN},
        .constants = {10_i32},
        .inOutCount = 2},
      fc::Chunk{.name = "f5", .code = {GET_VAL, 1, SET_VAL, 0, POP, RETURN}, .constants = {}, .inOutCount = 2}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(40, uut.viewStack().front().asI32());
}
