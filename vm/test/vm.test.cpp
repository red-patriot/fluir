#include "vm/vm.hpp"

#include <array>

#include <gtest/gtest.h>

namespace fc = fluir::code;
using enum fluir::code::OpCode;

TEST(TestVM, ExecEmptyFunction) {
  fluir::code::ByteCode code{.header = {},
                             .chunks = {
                                 fc::Chunk{
                                     .code = {
                                         EXIT}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
}

TEST(TestVM, ExecuteSimpleAddition) {
  fluir::code::ByteCode code{.header = {},
                             .chunks = {
                                 fc::Chunk{
                                     .code = {
                                         PUSH_FP, 0,
                                         PUSH_FP, 1,
                                         FP_ADD,
                                         EXIT},
                                     .constants = {1.2, 2.5}}}};
  double expected = 3.7;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back());
}

TEST(TestVM, ExecuteSimpleSubtraction) {
  fluir::code::ByteCode code{.header = {},
                             .chunks = {
                                 fc::Chunk{
                                     .code = {
                                         PUSH_FP, 0,
                                         PUSH_FP, 1,
                                         FP_SUBTRACT,
                                         EXIT},
                                     .constants = {1.2, 2.5}}}};
  double expected = -1.3;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back());
}

TEST(TestVM, ExecuteSimpleMultiply) {
  fluir::code::ByteCode code{.header = {},
                             .chunks = {
                                 fc::Chunk{
                                     .code = {
                                         PUSH_FP, 0,
                                         PUSH_FP, 1,
                                         FP_MULTIPLY,
                                         EXIT},
                                     .constants = {1.2, 2.5}}}};
  double expected = 3.0;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back());
}

TEST(TestVM, ExecuteSimpleDivide) {
  fluir::code::ByteCode code{.header = {},
                             .chunks = {
                                 fc::Chunk{
                                     .code = {
                                         PUSH_FP, 0,
                                         PUSH_FP, 1,
                                         FP_DIVIDE,
                                         EXIT},
                                     .constants = {1.2, 2.5}}}};
  double expected = 0.48;

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_DOUBLE_EQ(expected, uut.viewStack().back());
}

TEST(TestVM, PushAndPopCorrectly) {
  fluir::code::ByteCode code{.header = {},
                             .chunks = {
                                 fc::Chunk{
                                     .code = {
                                         PUSH_FP, 0,
                                         PUSH_FP, 1,
                                         FP_DIVIDE,
                                         POP,
                                         EXIT},
                                     .constants = {1.2, 2.5}}}};

  fluir::VirtualMachine uut;

  EXPECT_EQ(fluir::ExecResult::SUCCESS, uut.execute(&code));
  EXPECT_EQ(0, uut.viewStack().size());
}
