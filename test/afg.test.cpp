#include <gtest/gtest.h>

#include "fluir/compiler/afg_builder.hpp"
#include "fluir/compiler/parse_tree.hpp"

namespace fpt = fluir::parse_tree;  // codespell:ignore fpt
namespace fafg = fluir::afg;

TEST(TestBuildAFG, BuildGraphWithoutSharing) {
  auto block = fpt::Block{// codespell:ignore fpt
                          .nodes = {{1,
                                     fpt::Binary{// codespell:ignore fpt
                                                 .id = 1,
                                                 .lhs = 2,
                                                 .rhs = 3,
                                                 .op = fpt::Operator::PLUS,  // codespell:ignore fpt
                                                 .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7}}},
                                    {2,
                                     fpt::Constant{                              // codespell:ignore fpt
                                                   .value = fpt::FpDouble{1.0},  // codespell:ignore fpt
                                                   .id = 2,
                                                   .location = {.x = 0, .y = 10, .z = 2, .width = 5, .height = 5}}},
                                    {3,
                                     fpt::Constant{                              // codespell:ignore fpt
                                                   .value = fpt::FpDouble{2.0},  // codespell:ignore fpt
                                                   .id = 3,
                                                   .location = {.x = 0, .y = 10, .z = 3, .width = 5, .height = 5}}}}};

  fafg::FlowGraph expected;
  expected.add(std::make_unique<fafg::BinaryOp>(
      fluir::Operator::PLUS,
      fafg::makeDependency<fafg::DoubleConstant>(
          1.0,
          fluir::LocationInfo{.x = 0, .y = 10, .z = 2, .width = 5, .height = 5}),
      fafg::makeDependency<fafg::DoubleConstant>(
          2.0,
          fluir::LocationInfo{.x = 0, .y = 10, .z = 3, .width = 5, .height = 5}),
      fluir::LocationInfo{.x = 0, .y = 20, .z = 2, .width = 7, .height = 7}));

  auto actual = fluir::compiler::buildGraphOf(block);

  EXPECT_EQ(expected.topLevelCount(), actual.topLevelCount());
  EXPECT_TRUE(dynamic_cast<const fafg::BinaryOp*>(&actual.at(0)));
}
