#include "compiler/frontend/parser.hpp"
#include "compiler/frontend/parse_tree/parse_tree.hpp"

#include <string>
#include <tuple>
#include <variant>

#include <gtest/gtest.h>

using std::tuple;
using TestParserData = tuple<std::string, fluir::pt::ParseTree, std::string>;

class TestParser : public ::testing::TestWithParam<TestParserData> { };

TEST_P(TestParser, Test) {
  fluir::Context ctx;
  const auto& [source, expected, name] = GetParam();

  auto actual = fluir::parseString(ctx, source);

  EXPECT_FALSE(ctx.diagnostics.containsErrors());
  EXPECT_EQ(expected, actual.value());
}

namespace {
  TestParserData CanParseEmptyMain{
    R"(<?xml version="1.0" encoding="UTF-8"?>
      <fluir >
          <function
              name="main"
              id="1"
              x="10"
              y="10"
              z="3"
              w="100"
              h="100">
            <body></body>
          </function>
      </fluir>)",
    fluir::pt::ParseTree{
      .declarations = {{1,
                        fluir::pt::FunctionDecl{.id = 1,
                                                .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100},
                                                .name = "main",
                                                .body = fluir::pt::EMPTY_BLOCK}}}},
    "CanParseEmptyMain"};

  TestParserData CanParseSimpleBinaryExpression{
    R"(<?xml version="1.0" encoding="UTF-8"?>
         <fluir >
           <function
             name="main"
             id="1"
             x="10" y="10" z="3" w="100" h="100">
             <body>
               <binary
                 id="1"
                 x="0" y="20" z="2" w="7" h="7"
                 lhs="2"
                 rhs="3"
                 operator="-" />
                <constant
                 id="2"
                 x="5" y="5" z="0" w="5" h="5">
                 <float>5.6</float>
               </constant>
               <constant
                 id="3"
                 x="5" y="12" z="0" w="5" h="5">
                 <float>-4.7</float>
               </constant>
             </body>
           </function>
         </fluir>)",
    fluir::pt::ParseTree{
      .declarations =
        {{1,
          fluir::pt::FunctionDecl{
            .id = 1,
            .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100},
            .name = "main",
            .body = {.nodes = {{1,
                                fluir::pt::Binary{.id = 1,
                                                  .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7},
                                                  .lhs = 2,
                                                  .rhs = 3,
                                                  .op = fluir::Operator::MINUS}},
                               {2,
                                fluir::pt::Constant{.id = 2,
                                                    .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5},
                                                    .value = fluir::pt::Float{5.6}}},
                               {3,
                                fluir::pt::Constant{.id = 3,
                                                    .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                                    .value = fluir::pt::Float{-4.7}}}}}}}}},
    "CanParseSimpleBinaryExpression"};

  TestParserData CanParseSimpleUnaryExpression{
    R"(<?xml version="1.0" encoding="UTF-8"?>
        <fluir >
          <function
            name="main"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
              <unary
                id="3"
                x="0" y="20" z="2" w="7" h="7"
                lhs="2"
                operator="+" />
              <constant
                id="2"
                x="5" y="12" z="0" w="5" h="5">
                <float>12.4</float>
              </constant>
            </body>
          </function>
        </fluir>)",
    fluir::pt::ParseTree{
      .declarations =
        {{1,
          fluir::pt::FunctionDecl{
            .id = 1,
            .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100},
            .name = "main",
            .body = {.nodes = {{3,
                                fluir::pt::Unary{.id = 3,
                                                 .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7},
                                                 .lhs = 2,
                                                 .op = fluir::Operator::PLUS}},
                               {2,
                                fluir::pt::Constant{.id = 2,
                                                    .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                                    .value = fluir::pt::Float{12.4}}}}}}}}},
    "CanParseSimpleUnaryExpression"};

  TestParserData CanParseExpressionWithConduits{
    R"(<?xml version='1.0' encoding='UTF-8'?>
<fluir>
  <function name="main" id="1" x="1" y="5" z="3" w="100" h="100">
    <body>
      <binary id="3" x="35" y="3" z="2" w="5" h="5" operator="/"/>
      <constant id="2" x="9" y="12" z="10" w="12" h="5">
        <float>1.2345</float>
      </constant>
      <constant id="1" x="9" y="5" z="0" w="12" h="5">
        <float>6.7891</float>
      </constant>
      <binary id="5" x="57" y="4" z="2" w="5" h="5" operator="*"/>
      <unary id="6" x="35" y="12" z="0" w="5" h="5" operator="-"/>
      <conduit id="7" input="1">
        <output target="3" index="0"/>
      </conduit>
      <conduit id="8" input="2" index="0">
        <output target="3" index="1"/>
      </conduit>
      <conduit id="10" input="6">
        <output target="5" index="1"/>
      </conduit>
      <conduit id="11" input="2">
        <output target="6"/>
      </conduit>
      <conduit id="12" input="3">
        <output target="5" index="0"/>
      </conduit>
    </body>
  </function>
</fluir>)",
    fluir::pt::ParseTree{
      .declarations =
        {{1,
          fluir::pt::FunctionDecl{
            .id = 1,
            .location = {.x = 1, .y = 5, .z = 3, .width = 100, .height = 100},
            .name = "main",
            .body = {.nodes = {{3,
                                fluir::pt::Binary{.id = 3,
                                                  .location = {.x = 35, .y = 3, .z = 2, .width = 5, .height = 5},
                                                  .op = fluir::Operator::SLASH}},
                               {2,
                                fluir::pt::Constant{.id = 2,
                                                    .location = {.x = 9, .y = 12, .z = 10, .width = 12, .height = 5},
                                                    .value = fluir::pt::Float{1.2345}}},
                               {1,
                                fluir::pt::Constant{.id = 1,
                                                    .location = {.x = 9, .y = 5, .z = 0, .width = 12, .height = 5},
                                                    .value = fluir::pt::Float{6.7891}}},
                               {5,
                                fluir::pt::Binary{.id = 5,
                                                  .location = {.x = 57, .y = 4, .z = 2, .width = 5, .height = 5},
                                                  .op = fluir::Operator::STAR}},
                               {6,
                                fluir::pt::Unary{.id = 6,
                                                 .location = {.x = 35, .y = 12, .z = 0, .width = 5, .height = 5},
                                                 .op = fluir::Operator::MINUS}}},
                     .conduits =
                       {{7,
                         fluir::pt::Conduit{.id = 7,
                                            .input = 1,
                                            .index = 0,
                                            .children = {fluir::pt::Conduit::Output{.target = 3, .index = 0}}}},
                        {8,
                         fluir::pt::Conduit{.id = 8,
                                            .input = 2,
                                            .index = 0,
                                            .children = {fluir::pt::Conduit::Output{.target = 3, .index = 1}}}},
                        {10,
                         fluir::pt::Conduit{.id = 10,
                                            .input = 6,
                                            .index = 0,
                                            .children = {fluir::pt::Conduit::Output{.target = 5, .index = 1}}}},
                        {11,
                         fluir::pt::Conduit{
                           .id = 11, .input = 2, .children = {fluir::pt::Conduit::Output{.target = 6, .index = 0}}}},
                        {12,
                         fluir::pt::Conduit{.id = 12,
                                            .input = 3,
                                            .children = {fluir::pt::Conduit::Output{.target = 5, .index = 0}}}}}}}}}},
    "CanParseExpressionWithConduits"};
}  // namespace

INSTANTIATE_TEST_SUITE_P(,
                         TestParser,
                         ::testing::Values(CanParseEmptyMain,
                                           CanParseSimpleBinaryExpression,
                                           CanParseSimpleUnaryExpression,
                                           CanParseExpressionWithConduits),
                         [](const ::testing::TestParamInfo<TestParser::ParamType>& info) {
                           return std::get<2>(info.param);
                         });
