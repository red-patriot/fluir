#include "compiler/frontend/parser.hpp"

#include <string>

#include <gtest/gtest.h>

TEST(TestParser, CanParseEmptyMain) {
  std::string source = R"(<?xml version="1.0" encoding="UTF-8"?>
<fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
    <fl:function
        name="main"
        id="1"
        x="10"
        y="10"
        z="3"
        w="100"
        h="100">
      <body></body>
    </fl:function>
</fluir>)";

  fluir::pt::ParseTree expected{
      .declarations = {{1,
                        fluir::pt::FunctionDecl{
                            .location = {.x = 10,
                                         .y = 10,
                                         .z = 3,
                                         .width = 100,
                                         .height = 100},
                            .name = "main",
                            .body = fluir::pt::EMPTY_BLOCK}}}};

  auto actual = fluir::parseString(source);

  EXPECT_FALSE(actual.containsErrors());
  EXPECT_EQ(expected, actual.value());
}

TEST(TestParser, CanParseSimpleBinaryExpression) {
  std::string source = R"(<?xml version="1.0" encoding="UTF-8"?>
<fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
  <fl:function
    name="main"
    id="1"
    x="10" y="10" z="3" w="100" h="100">
    <body>
      <fl:binary
        id="1"
        x="0" y="20" z="2" w="7" h="7"
        lhs="2"
        rhs="3"
        operator="-" />
      <fl:constant
        id="2"
        x="5" y="5" z="0" w="5" h="5">
        <fl:float>5.6</fl:float>
      </fl:constant>
      <fl:constant
        id="3"
        x="5" y="12" z="0" w="5" h="5">
        <fl:float>-4.7</fl:float>
      </fl:constant>
    </body>
  </fl:function>
</fluir>)";

  fluir::pt::ParseTree expected{
      .declarations = {{1,
                        fluir::pt::FunctionDecl{
                            .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100},
                            .name = "main",
                            .body = {
                                {1,
                                 fluir::pt::Binary{
                                     .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7},
                                     .lhs = 2,
                                     .rhs = 3,
                                     .op = fluir::Operator::MINUS}},
                                {2,
                                 fluir::pt::Constant{
                                     .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5},
                                     .value = fluir::pt::Float{5.6}}},
                                {3,
                                 fluir::pt::Constant{
                                     .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                     .value = fluir::pt::Float{-4.7}}}}}}}};

  auto actual = fluir::parseString(source);

  EXPECT_FALSE(actual.containsErrors());
  EXPECT_EQ(expected, actual.value());
}
