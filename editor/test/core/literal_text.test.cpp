#include "editor/core/literal_text.hpp"

#include <optional>
#include <string>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"

namespace {

  using fluir::editor::isEditableLiteral;
  using fluir::editor::isValid;
  using fluir::editor::literalTypeName;
  using fluir::editor::renderLiteral;
  using fluir::pt::Constant;

  TEST(LiteralText, RendersEachIntegralAlternativeAsTheNodeLabelDoes) {
    EXPECT_EQ(renderLiteral((Constant{.value = "-12", .typeTag = Constant::I8})), "-12");
    EXPECT_EQ(renderLiteral((Constant{.value = "-1200", .typeTag = Constant::I16})), "-1200");
    EXPECT_EQ(renderLiteral((Constant{.value = "-120000", .typeTag = Constant::I32})), "-120000");
    EXPECT_EQ(renderLiteral((Constant{.value = "-1200000000", .typeTag = Constant::I64})), "-1200000000");
    EXPECT_EQ(renderLiteral((Constant{.value = "12", .typeTag = Constant::U8})), "12");
    EXPECT_EQ(renderLiteral((Constant{.value = "1200", .typeTag = Constant::U16})), "1200");
    EXPECT_EQ(renderLiteral((Constant{.value = "120000", .typeTag = Constant::U32})), "120000");
    EXPECT_EQ(renderLiteral((Constant{.value = "1200000000", .typeTag = Constant::U64})), "1200000000");
  }

  TEST(LiteralText, NamesEachAlternativeByItsSourceTag) {
    EXPECT_EQ(literalTypeName((Constant{.value = "1.5", .typeTag = Constant::F64})), "F64");
    EXPECT_EQ(literalTypeName((Constant{.value = "1", .typeTag = Constant::I8})), "I8");
    EXPECT_EQ(literalTypeName((Constant{.value = "1", .typeTag = Constant::I16})), "I16");
    EXPECT_EQ(literalTypeName((Constant{.value = "1", .typeTag = Constant::I32})), "I32");
    EXPECT_EQ(literalTypeName((Constant{.value = "1", .typeTag = Constant::I64})), "I64");
    EXPECT_EQ(literalTypeName((Constant{.value = "1", .typeTag = Constant::U8})), "U8");
    EXPECT_EQ(literalTypeName((Constant{.value = "1", .typeTag = Constant::U16})), "U16");
    EXPECT_EQ(literalTypeName((Constant{.value = "1", .typeTag = Constant::U32})), "U32");
    EXPECT_EQ(literalTypeName((Constant{.value = "1", .typeTag = Constant::U64})), "U64");
    EXPECT_EQ(literalTypeName((Constant{.value = "true", .typeTag = Constant::BOOL})), "BOOL");
  }

  TEST(LiteralText, RendersNamedConstants) {
    EXPECT_EQ(renderLiteral((Constant{.value = "pi", .typeTag = Constant::F64})), "\u03C0");        // π
    EXPECT_EQ(renderLiteral((Constant{.value = "e", .typeTag = Constant::F64})), "\U0001D486");     // 𝒆
    EXPECT_EQ(renderLiteral((Constant{.value = "ln(2)", .typeTag = Constant::F64})), "\u33D12");    // ㏑2
    EXPECT_EQ(renderLiteral((Constant{.value = "ln(10)", .typeTag = Constant::F64})), "\u33D110");  // ㏑10
    EXPECT_EQ(renderLiteral((Constant{.value = "sqrt(2)", .typeTag = Constant::F64})), "\u221A2");  // √2
    EXPECT_EQ(renderLiteral((Constant{.value = "sqrt(3)", .typeTag = Constant::F64})), "\u221A3");  // √3
    EXPECT_EQ(renderLiteral((Constant{.value = "inf", .typeTag = Constant::F64})), "\u221E");       // ∞
  }

  TEST(LiteralText, RejectsI32TextThatIsNotANumber) {
    const Constant like{.value = "0", .typeTag = Constant::I32};
    EXPECT_FALSE(isValid(like.typeTag, "abc"));
  }

  TEST(LiteralText, RejectsUnsignedTextWithNegative) {
    const Constant likeU8{.value = "0", .typeTag = Constant::U8};
    EXPECT_FALSE(isValid(likeU8.typeTag, "-300"));
  }

  TEST(LiteralText, RejectsAnEmptyNumber) {
    const Constant likeI32{.value = "0", .typeTag = Constant::I32};
    EXPECT_FALSE(isValid(likeI32.typeTag, ""));
    const Constant likeU32{.value = "0", .typeTag = Constant::U32};
    EXPECT_FALSE(isValid(likeI32.typeTag, ""));
    const Constant likeF64{.value = "0", .typeTag = Constant::F64};
    EXPECT_FALSE(isValid(likeI32.typeTag, ""));
  }

  TEST(LiteralText, AllowsAllStrText) {
    const Constant like{.value = "asdf", .typeTag = Constant::STR};
    EXPECT_TRUE(isValid(like.typeTag, "abc"));
    EXPECT_TRUE(isValid(like.typeTag, ""));
    EXPECT_TRUE(isValid(like.typeTag, "asdf\nasdf\r\n"));
    EXPECT_TRUE(isValid(like.typeTag, "\0"));
  }

  TEST(LiteralText, F64ConstantsAreEditable) {
    const Constant like{.value = "1.5", .typeTag = Constant::F64};
    EXPECT_TRUE(isEditableLiteral(like));
  }

  TEST(LiteralText, StrConstantsAreEditable) {
    const Constant like{.value = "Hello World!", .typeTag = Constant::STR};
    EXPECT_TRUE(isEditableLiteral(like));
  }

  // TEST(LiteralText, RejectsMalformedOrNonFiniteDouble) {
  //   const Constant like{.value="1.5", .typeTag=Constant::F64};
  //   for (const char* text : {"", "abc", "1.5x", "1e999", "inf", "nan"}) {
  //     EXPECT_EQ(tryParseLiteral(like, text), std::nullopt) << text;
  //   }
  // }

  TEST(LiteralText, BoolConstantsAreNotEditable) {
    const Constant like{.value = "true", .typeTag = Constant::BOOL};

    EXPECT_FALSE(isEditableLiteral(like));
  }

}  // namespace
