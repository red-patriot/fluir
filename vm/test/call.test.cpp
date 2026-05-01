#include <gtest/gtest.h>

#include "vm/machine/vm.hpp"

namespace fc = fluir::code;
using enum fluir::code::Instruction;
using namespace fluir::code::value_literals;

TEST(TestVMFunctionCalls, HandlesCallWithoutParamsWithReturn) {
  std::int32_t expected = 4;

  fluir::code::ByteCode code{
    .header = {.entryOffset = 0},
    .constants = {4_i32},
    .chunks = {fc::Chunk{.name = "main", .code = {RESERVE, 1, CALL, 0, 0, 0, 1, EXIT}},
               fc::Chunk{.name = "getVal", .code = {PUSH, 0, SET_VAL, 0, POP, RETURN}, .inCount = 0, .outCount = 1}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(expected, uut.viewStack().front().asI32());
}

TEST(TestVMFunctionCalls, HandlesCallWithParamsWithReturn) {
  fluir::code::ByteCode code{
    .header = {.entryOffset = 0},
    .constants = {3_i32, 7_i32},
    .chunks = {fc::Chunk{.name = "main", .code = {RESERVE, 1, PUSH, 0, PUSH, 1, CALL, 0, 0, 0, 1, EXIT}},
               fc::Chunk{.name = "add",
                         .code = {GET_VAL, 1, GET_VAL, 2, I64_ADD, SET_VAL, 0, POP, RETURN},
                         .inCount = 2,
                         .outCount = 1}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(10, uut.viewStack().front().asI32());
}

TEST(TestVMFunctionCalls, HandlesCallWithoutParamsWithoutReturn) {
  fluir::code::ByteCode code{.header = {.entryOffset = 0},
                             .constants = {},
                             .chunks = {fc::Chunk{.name = "main", .code = {CALL, 0, 0, 0, 1, EXIT}},
                                        fc::Chunk{.name = "noop", .code = {RETURN}, .inCount = 0, .outCount = 0}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_TRUE(uut.viewStack().empty());
}

TEST(TestVMFunctionCalls, HandlesCallWithParamsWithoutReturn) {
  fluir::code::ByteCode code{.header = {.entryOffset = 0},
                             .constants = {3_i32, 7_i32},
                             .chunks = {fc::Chunk{.name = "main", .code = {PUSH, 0, PUSH, 1, CALL, 0, 0, 0, 1, EXIT}},
                                        fc::Chunk{.name = "sink", .code = {RETURN}, .inCount = 2, .outCount = 0}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_TRUE(uut.viewStack().empty());
}

TEST(TestVMFunctionCalls, HandlesDeepCallStack) {
  fluir::code::ByteCode code{
    .header = {.entryOffset = 0},
    .constants = {0_i32, 10_i32},
    .chunks = {fc::Chunk{.name = "main", .code = {RESERVE, 1, PUSH, 0, CALL, 0, 0, 0, 1, EXIT}},
               fc::Chunk{.name = "f1",
                         .code = {RESERVE, 1, GET_VAL, 1, PUSH, 1, I64_ADD, CALL, 0, 0, 0, 2, SET_VAL, 0, POP, RETURN},
                         .inCount = 1,
                         .outCount = 1},
               fc::Chunk{.name = "f2",
                         .code = {RESERVE, 1, GET_VAL, 1, PUSH, 1, I64_ADD, CALL, 0, 0, 0, 3, SET_VAL, 0, POP, RETURN},
                         .inCount = 1,
                         .outCount = 1},
               fc::Chunk{.name = "f3",
                         .code = {RESERVE, 1, GET_VAL, 1, PUSH, 1, I64_ADD, CALL, 0, 0, 0, 4, SET_VAL, 0, POP, RETURN},
                         .inCount = 1,
                         .outCount = 1},
               fc::Chunk{.name = "f4",
                         .code = {RESERVE, 1, GET_VAL, 1, PUSH, 1, I64_ADD, CALL, 0, 0, 0, 5, SET_VAL, 0, POP, RETURN},
                         .inCount = 1,
                         .outCount = 1},
               fc::Chunk{.name = "f5", .code = {GET_VAL, 1, SET_VAL, 0, POP, RETURN}, .inCount = 1, .outCount = 1}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(40, uut.viewStack().front().asI32());
}

TEST(TestVMFunctionCalls, HandlesMainWithReturn) {
  std::int32_t expected = 4;

  fluir::code::ByteCode code{
    .header = {.entryOffset = 0}, .constants = {}, .chunks = {fc::Chunk{.name = "main", .code = {RETURN}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
}
