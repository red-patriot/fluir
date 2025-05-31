#include "compiler/frontend/parser.hpp"

#include <string>
#include <tuple>

#include <gtest/gtest.h>

using std::tuple;
using TestParserData = tuple<std::string,
                             fluir::pt::ParseTree,
                             std::string>;

class TestParser : public ::testing::TestWithParam<TestParserData> { };

TEST_P(TestParser, Test) {
  const auto& [source, expected, name] = GetParam();

  auto actual = fluir::parseString(source);

  EXPECT_FALSE(actual.containsErrors());
  EXPECT_EQ(expected, actual.value());
}

namespace {
  TestParserData CanParseEmptyMain{
      R"(<?xml version="1.0" encoding="UTF-8"?>
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
      </fluir>)",
      fluir::pt::ParseTree{
          .declarations = {{1,
                            fluir::pt::FunctionDecl{
                                .id = 1,
                                .location = {.x = 10,
                                             .y = 10,
                                             .z = 3,
                                             .width = 100,
                                             .height = 100},
                                .name = "main",
                                .body = fluir::pt::EMPTY_BLOCK}}}},
      "CanParseEmptyMain"};

  TestParserData CanParseSimpleBinaryExpression{
      R"(<?xml version="1.0" encoding="UTF-8"?>
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
         </fluir>)",
      fluir::pt::ParseTree{
          .declarations = {
              {1,
               fluir::pt::FunctionDecl{
                   .id = 1,
                   .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100},
                   .name = "main",
                   .body = {
                       .nodes = {
                           {1,
                            fluir::pt::Binary{
                                .id = 1,
                                .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7},
                                .lhs = 2,
                                .rhs = 3,
                                .op = fluir::Operator::MINUS}},
                           {2,
                            fluir::pt::Constant{
                                .id = 2,
                                .location = {.x = 5, .y = 5, .z = 0, .width = 5, .height = 5},
                                .value = fluir::pt::Float{5.6}}},
                           {3,
                            fluir::pt::Constant{
                                .id = 3,
                                .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                .value = fluir::pt::Float{-4.7}}}}}}}}},
      "CanParseSimpleBinaryExpression"};

  TestParserData CanParseSimpleUnaryExpression{
      R"(<?xml version="1.0" encoding="UTF-8"?>
        <fluir xmlns:fl="FLUIR::LANGUAGE::SOURCE">
          <fl:function
            name="main"
            id="1"
            x="10" y="10" z="3" w="100" h="100">
            <body>
              <fl:unary
                id="3"
                x="0" y="20" z="2" w="7" h="7"
                lhs="2"
                operator="+" />
              <fl:constant
                id="2"
                x="5" y="12" z="0" w="5" h="5">
                <fl:float>12.4</fl:float>
              </fl:constant>
            </body>
          </fl:function>
        </fluir>)",
      fluir::pt::ParseTree{
          .declarations = {
              {1,
               fluir::pt::FunctionDecl{
                   .id = 1,
                   .location = {.x = 10, .y = 10, .z = 3, .width = 100, .height = 100},
                   .name = "main",
                   .body = {
                       .nodes = {
                           {3,
                            fluir::pt::Unary{
                                .id = 3,
                                .location = {.x = 0, .y = 20, .z = 2, .width = 7, .height = 7},
                                .lhs = 2,
                                .op = fluir::Operator::PLUS}},
                           {2,
                            fluir::pt::Constant{
                                .id = 2,
                                .location = {.x = 5, .y = 12, .z = 0, .width = 5, .height = 5},
                                .value = fluir::pt::Float{12.4}}}}}}}}},
      "CanParseSimpleUnaryExpression"};
}  // namespace

INSTANTIATE_TEST_SUITE_P(, TestParser,
                         ::testing::Values(CanParseEmptyMain,
                                           CanParseSimpleBinaryExpression,
                                           CanParseSimpleUnaryExpression),
                         [](const ::testing::TestParamInfo<TestParser::ParamType>& info) {
                           return std::get<2>(info.param);
                         });
