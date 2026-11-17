#include "editor/core/literal_text.hpp"

#include <optional>
#include <string>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"

namespace {

  using fluir::editor::isEditableLiteral;
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

  // TODO: Reenable or delete these tests

  // TEST(LiteralText, ParsesTextBackIntoTheSameAlternative) {
  //   const Constant like{.value="0", .typeTag=Constant::I16};
  //   const std::optional<Literal> parsed = tryParseLiteral(like, "42");
  //   ASSERT_TRUE(parsed.has_value());
  //   EXPECT_EQ(parsed->index(), like.index());
  //   EXPECT_EQ(std::get<I16>(*parsed), 42);
  // }
  //
  // TEST(LiteralText, RejectsTextThatIsNotANumber) {
  //   const Constant like{.value="0", .typeTag=Constant::I32};
  //   EXPECT_EQ(tryParseLiteral(like, "abc"), std::nullopt);
  // }
  //
  // TEST(LiteralText, RejectsAValueOutsideTheAlternativesRange) {
  //   const Literal likeI8{std::in_place_type<I8>, I8{0}};
  //   EXPECT_EQ(tryParseLiteral(likeI8, "300"), std::nullopt);
  //
  //   const Literal likeU8{std::in_place_type<U8>, U8{0}};
  //   EXPECT_EQ(tryParseLiteral(likeU8, "-1"), std::nullopt);
  // }
  //
  // TEST(LiteralText, RejectsAnEmptyDraft) {
  //   const Constant like{.value="0", .typeTag=Constant::I32};
  //   EXPECT_EQ(tryParseLiteral(like, ""), std::nullopt);
  // }

  TEST(LiteralText, DoubleConstantsAreEditable) {
    const Constant like{.value = "1.5", .typeTag = Constant::F64};
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
