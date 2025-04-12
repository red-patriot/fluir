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
                                             .nodes = {{1,
                                                        fa::Constant{
                                                            .value = fa::FpDouble{6.7},
                                                            .id = 1,
                                                            .location = {.x = 0, .y = 10, .z = 3, .width = 5, .height = 5}}}}},
                                         .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100}}}}};

  fluir::compiler::Parser uut;

  auto actual = uut.parse(src);

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

  auto actual = uut.parse(src);

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
