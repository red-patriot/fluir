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
