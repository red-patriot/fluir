#include "vm/vm.hpp"

#include <array>

#include <gtest/gtest.h>

namespace fc = fluir::code;
using enum fluir::code::Instruction;
using namespace fluir::code::value_literals;

TEST(TestVM, ExecEmptyFunction) {
  fluir::code::ByteCode code{.header = {}, .chunks = {fc::Chunk{.code = {EXIT}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
}

TEST(TestVM, ExecuteSimpleAddition) {
  fluir::code::ByteCode code{
    .header = {},
    .chunks = {fc::Chunk{.code = {PUSH_F64, 0, PUSH_F64, 1, F64_ADD, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};
  double expected = 3.7;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back().asF64());
}

TEST(TestVM, ExecuteSimpleSubtraction) {
  fluir::code::ByteCode code{
    .header = {},
    .chunks = {fc::Chunk{.code = {PUSH_F64, 0, PUSH_F64, 1, F64_SUB, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};
  double expected = -1.3;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back().asF64());
}

TEST(TestVM, ExecuteSimpleMultiply) {
  fluir::code::ByteCode code{
    .header = {},
    .chunks = {fc::Chunk{.code = {PUSH_F64, 0, PUSH_F64, 1, F64_MUL, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};
  double expected = 3.0;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back().asF64());
}

TEST(TestVM, ExecuteSimpleDivide) {
  fluir::code::ByteCode code{
    .header = {},
    .chunks = {fc::Chunk{.code = {PUSH_F64, 0, PUSH_F64, 1, F64_DIV, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};
  double expected = 0.48;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back().asF64());
}

TEST(TestVM, PushAndPopCorrectly) {
  fluir::code::ByteCode code{
    .header = {},
    .chunks = {fc::Chunk{.code = {PUSH_F64, 0, PUSH_F64, 1, F64_DIV, POP, EXIT}, .constants = {1.2_f64, 2.5_f64}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(0, uut.viewStack().size());
}
