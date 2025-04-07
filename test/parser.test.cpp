#include "fluir/compiler/parser.hpp"

#include <string>

#include <gtest/gtest.h>

#include "fluir/compiler/ast.hpp"

namespace fa = fluir::ast;

TEST(TestParser, CanParseEmptyMainFunction) {
  std::string src = R"(<?xml version="1.0" encoding="UTF-8"?>
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

  fa::AST expected{
      .declarations = {{1,
                        fa::FunctionDecl{.name = "main",
                                         .id = 1,
                                         .body = fa::EMPTY_BLOCK,
                                         .location = {.x = 10,
                                                      .y = 10,
                                                      .z = 3,
                                                      .width = 100,
                                                      .height = 100}}}}};

  fluir::compiler::Parser uut;

  auto actual = uut.parse(src);

  EXPECT_TRUE(uut.diagnostics().empty());
  EXPECT_EQ(expected, actual);
}

TEST(TestParser, ParseSingleConstant) {
  std::string src = R"(<?xml version="1.0" encoding="UTF-8"?>
<fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
    <fl:function
        name="main"
        id="1"
        x="10"
        y="10"
        z="3"
        w="100"
        h="100">
      <body>
        <fl:constant
           id="1"
            x="0"
            y="10"
            z="3"
            w="5"
            h="5">
          <fl:double>6.7</fl:double>
        </fl:constant>
      </body>
    </fl:function>
</fluir>)";

  fa::AST expected{
      .declarations = {{1,
                        fa::FunctionDecl{.name = "main",
                                         .id = 1,
                                         .body = fa::Block{
                                             .node = fa::Constant{
                                                 .value = fa::FpDouble{6.7},
                                                 .id = 1,
                                                 .location = {.x = 0, .y = 10, .z = 3, .width = 5, .height = 5}}},
                                         .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100}}}}};

  fluir::compiler::Parser uut;

  auto actual = uut.parse(src);

  EXPECT_TRUE(uut.diagnostics().empty());
  EXPECT_EQ(expected, actual);
}
