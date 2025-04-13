#include "fluir/compiler/parser.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "fluir/compiler/ast.hpp"

namespace fa = fluir::ast;

namespace fs = std::filesystem;

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

  auto actual = uut.parseString(src);

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
                                             .nodes = {{1,
                                                        fa::Constant{
                                                            .value = fa::FpDouble{6.7},
                                                            .id = 1,
                                                            .location = {.x = 0, .y = 10, .z = 3, .width = 5, .height = 5}}}}},
                                         .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100}}}}};

  fluir::compiler::Parser uut;

  auto actual = uut.parseString(src);

  EXPECT_TRUE(uut.diagnostics().empty());
  EXPECT_EQ(expected, actual);
}

TEST(TestParser, ParseSimpleBinaryExpression) {
  std::string src = R"(<?xml version="1.0" encoding="UTF-8"?>
<fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
    <fl:function
        name="main"
        id="1"
        x="15"
        y="7"
        z="3"
        w="100"
        h="100">
      <body>
        <fl:binary
           id="1"
           x="0"
           y="20"
           z="2"
           w="7"
           h="7"
           lhs="2"
           rhs="3"
           operator="-" />
         <fl:constant
           id="2"
           x="0"
           y="10"
           z="2"
           w="5"
           h="5">
          <fl:double>1.0</fl:double>
        </fl:constant>
        <fl:constant
           id="3"
           x="0"
           y="10"
           z="3"
           w="5"
           h="5">
          <fl:double>2.0</fl:double>
        </fl:constant>
      </body>
    </fl:function>
</fluir>)";

  fa::AST expected{
      .declarations = {{1,
                        fa::FunctionDecl{.name = "main",
                                         .id = 1,
                                         .body = fa::Block{
                                             .nodes = {{1,
                                                        fa::Binary{
                                                            .id = 1,
                                                            .lhs = 2,
                                                            .rhs = 3,
                                                            .op = fa::Operator::MINUS,
                                                            .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7}}},
                                                       {2,
                                                        fa::Constant{
                                                            .value = fa::FpDouble{1.0},
                                                            .id = 2,
                                                            .location = {.x = 0, .y = 10, .z = 2, .width = 5, .height = 5}}},
                                                       {3,
                                                        fa::Constant{
                                                            .value = fa::FpDouble{2.0},
                                                            .id = 3,
                                                            .location = {.x = 0, .y = 10, .z = 3, .width = 5, .height = 5}}}}},
                                         .location = {.x = 15, .y = 7, .z = 3, .width = 100, .height = 100}}}}};

  fluir::compiler::Parser uut;

  auto actual = uut.parseString(src);

  EXPECT_TRUE(uut.diagnostics().empty());
  EXPECT_EQ(expected, actual);
}

TEST(TestParser, ParseComplexBinaryUnaryExpression) {
  std::string src = R"(<?xml version="1.0" encoding="UTF-8"?>
<fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
    <fl:function name="main" id="1"
        x="15" y="7" z="3" w="100" h="100">
        <body>
            <fl:constant id="1"
                x="2" y="20" z="1" w="5" h="5">
                <fl:double>3.0</fl:double>
            </fl:constant>
            <fl:constant id="4"
                x="2" y="12" z="1" w="5" h="5">
                <fl:double>2.0</fl:double>
            </fl:constant>
            <fl:constant id="6"
                x="14" y="2" z="1" w="7" h="5">
                <fl:double>7.5</fl:double>
            </fl:constant>
            <fl:binary id="2"
                x="16" y="18" z="1" w="4" h="4"
                lhs="1" rhs="4" operator="*" />
            <fl:binary id="5"
                x="22" y="10" z="1" w="4" h="4"
                lhs="2" rhs="6" operator="/" />
            <fl:unary id="3"
                x="33" y="16" z="1" w="4" h="4"
                lhs="5" operator="-" />
        </body>
    </fl:function>
</fluir>)";

  fa::AST expected{
      .declarations = {{1,
                        fa::FunctionDecl{.name = "main",
                                         .id = 1,
                                         .body = fa::Block{
                                             .nodes = {
                                                 {1,
                                                  fa::Constant{
                                                      .value = 3.0,
                                                      .id = 1,
                                                      .location = {.x = 2, .y = 20, .z = 1, .width = 5, .height = 5}}},
                                                 {4,
                                                  fa::Constant{
                                                      .value = 2.0,
                                                      .id = 4,
                                                      .location = {.x = 2, .y = 12, .z = 1, .width = 5, .height = 5}}},
                                                 {6,
                                                  fa::Constant{
                                                      .value = 7.5,
                                                      .id = 6,
                                                      .location = {.x = 14, .y = 2, .z = 1, .width = 7, .height = 5}}},
                                                 {2,
                                                  fa::Binary{
                                                      .id = 2,
                                                      .lhs = 1,
                                                      .rhs = 4,
                                                      .op = fa::Operator::STAR,
                                                      .location = {.x = 16, .y = 18, .z = 1, .width = 4, .height = 4}}},
                                                 {5,
                                                  fa::Binary{
                                                      .id = 5,
                                                      .lhs = 2,
                                                      .rhs = 6,
                                                      .op = fa::Operator::SLASH,
                                                      .location = {.x = 22, .y = 10, .z = 1, .width = 4, .height = 4}}},
                                                 {3,
                                                  fa::Unary{
                                                      .id = 3,
                                                      .lhs = 5,
                                                      .op = fa::Operator::MINUS,
                                                      .location = {.x = 33, .y = 16, .z = 1, .width = 4, .height = 4}}}}},
                                         .location = {.x = 15, .y = 7, .z = 3, .width = 100, .height = 100}}}}};

  fluir::compiler::Parser uut;

  auto actual = uut.parseString(src);

  EXPECT_TRUE(uut.diagnostics().empty());
  EXPECT_EQ(expected, actual);
}

class TestParserErrors : public ::testing::TestWithParam<std::filesystem::path> {
 public:
  std::string readErrors(const fs::path& file) {
    std::fstream fin(file);
    std::stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
  }
};

TEST_P(TestParserErrors, Test) {
  const auto programFile = GetParam();
  const auto errorsFile = fs::path{programFile}.replace_extension(".errors");
  const auto errors = readErrors(errorsFile);

  fluir::compiler::Parser uut;

  uut.parseFile(programFile);

  std::stringstream ss;
  for (const auto& diagnostic : uut.diagnostics()) {
    ss << diagnostic << '\n';
  }

  auto actual = ss.str();

  EXPECT_EQ(errors, actual);
}

static std::vector<fs::path> getTestPrograms(const fs::path& parent) {
  std::vector<fs::path> programs;

  for (const auto& entry : fs::directory_iterator(parent)) {
    if (fs::is_regular_file(entry) && fs::path(entry).extension() == ".fl") {
      programs.emplace_back(entry.path());
    }
  }

  return programs;
}

INSTANTIATE_TEST_SUITE_P(TestParserErrors, TestParserErrors,
                         ::testing::ValuesIn(getTestPrograms(TEST_FOLDER / "programs" / "parser_error")),
                         [](const ::testing::TestParamInfo<fs::path>& info) {
                           return info.param.stem().string();
                         });
