#include "fluir/compiler/parser.hpp"

#include <string>

#include <gtest/gtest.h>

#include "fluir/compiler/ast.hpp"

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

  fluir::ast::AST expected{
      .declarations = {{1,
                        fluir::ast::FunctionDecl{.name = "main",
                                                 .id = 1,
                                                 .body = fluir::ast::EMPTY_BLOCK,
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
