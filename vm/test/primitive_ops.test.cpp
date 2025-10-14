#include <cmath>
#include <tuple>

#include <gtest/gtest.h>

#include "vm/vm.hpp"

using std::tuple;

namespace fc = fluir::code;
using enum fluir::code::Instruction;
using namespace fluir::code::value_literals;

class TestPrimitiveOps : public ::testing::TestWithParam<tuple<fc::Value, fc::Chunk>> { };

TEST_P(TestPrimitiveOps, Test) {
  auto [expected, chunk] = GetParam();
  // Add postamble to bytecode to make it execute correctly without requiring too much boilerplate in tests
  chunk.code.push_back(EXIT);
  fc::ByteCode code{.header = {}, .chunks = {std::move(chunk)}};

  fluir::VirtualMachine vm;
  EXPECT_EQ(fluir::ExecResult::SUCCESS, vm.execute(&code));
  auto actual = vm.viewStack().back();
  if (actual.type() == fc::PrimitiveType::F64) {
    // stupid double epsilon issues
    EXPECT_DOUBLE_EQ(expected.asF64(), actual.asF64());
  } else {
    EXPECT_EQ(expected, actual);
  }
}

INSTANTIATE_TEST_SUITE_P(
  TestPrimitiveOps,
  TestPrimitiveOps,
  ::testing::Values(
    tuple{3.5_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_ADD}, .constants = {1.5_f64, 2.0_f64}}},
    tuple{1.25_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_SUB}, .constants = {1.5_f64, 0.25_f64}}},
    tuple{12.0_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {1.5_f64, 8.0_f64}}},
    tuple{0.75_f64, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {3.0_f64, 4.0_f64}}},
    tuple{fc::Value{INFINITY}, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {1.0e200_f64, 2.0e200_f64}}},
    tuple{fc::Value{INFINITY}, fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {3.0_f64, 0.0_f64}}}));

// Test this separately since NAN isn't equal to NAN
class TestPrimitiveOpsProducingNans : public ::testing::TestWithParam<fc::Chunk> { };
TEST_P(TestPrimitiveOpsProducingNans, Test0Div) {
  auto chunk = GetParam();
  chunk.code.push_back(EXIT);
  // With NANs, compare slightly differently
  fc::ByteCode code{.header = {}, .chunks = {std::move(chunk)}};

  fluir::VirtualMachine vm;
  EXPECT_EQ(fluir::ExecResult::SUCCESS, vm.execute(&code));
  auto actual = vm.viewStack().back();
  EXPECT_TRUE(std::isnan(actual.asF64()));
}

static const auto NAN_FL_VALUE = fc::Value{NAN};

INSTANTIATE_TEST_SUITE_P(
  TestPrimitiveOpsProducingNans,
  TestPrimitiveOpsProducingNans,
  ::testing::Values(fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {0.0_f64, 0.0_f64}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_ADD}, .constants = {1.0_f64, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_SUB}, .constants = {2.0_f64, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {5.0_f64, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {3.0_f64, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_ADD}, .constants = {NAN_FL_VALUE, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_SUB}, .constants = {NAN_FL_VALUE, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_MUL}, .constants = {NAN_FL_VALUE, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, PUSH, 1, F64_DIV}, .constants = {NAN_FL_VALUE, NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, F64_NEG}, .constants = {NAN_FL_VALUE}},
                    fc::Chunk{.code = {PUSH, 0, F64_AFF}, .constants = {NAN_FL_VALUE}}));
