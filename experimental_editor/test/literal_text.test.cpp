#include "editor/core/literal_text.hpp"

#include <optional>
#include <string>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"

namespace {

  using namespace fluir::literals_types;
  using fluir::editor::isEditableLiteral;
  using fluir::editor::renderLiteral;
  using fluir::editor::tryParseLiteral;
  using fluir::pt::Literal;

  TEST(LiteralText, RendersEachIntegralAlternativeAsTheNodeLabelDoes) {
    EXPECT_EQ(renderLiteral(Literal(std::in_place_type<I8>, -12)), "-12");
    EXPECT_EQ(renderLiteral(Literal(std::in_place_type<I16>, -1200)), "-1200");
    EXPECT_EQ(renderLiteral(Literal(std::in_place_type<I32>, -120000)), "-120000");
    EXPECT_EQ(renderLiteral(Literal(std::in_place_type<I64>, -1200000000)), "-1200000000");
    EXPECT_EQ(renderLiteral(Literal(std::in_place_type<U8>, 12)), "12");
    EXPECT_EQ(renderLiteral(Literal(std::in_place_type<U16>, 1200)), "1200");
    EXPECT_EQ(renderLiteral(Literal(std::in_place_type<U32>, 120000)), "120000");
    EXPECT_EQ(renderLiteral(Literal(std::in_place_type<U64>, 1200000000)), "1200000000");
  }

  TEST(LiteralText, ParsesTextBackIntoTheSameAlternative) {
    const Literal like{std::in_place_type<I16>, I16{0}};
    const std::optional<Literal> parsed = tryParseLiteral(like, "42");
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->index(), like.index());
    EXPECT_EQ(std::get<I16>(*parsed), 42);
  }

  TEST(LiteralText, RejectsTextThatIsNotANumber) {
    const Literal like{std::in_place_type<I32>, I32{0}};
    EXPECT_EQ(tryParseLiteral(like, "abc"), std::nullopt);
  }

  TEST(LiteralText, RejectsAValueOutsideTheAlternativesRange) {
    const Literal likeI8{std::in_place_type<I8>, I8{0}};
    EXPECT_EQ(tryParseLiteral(likeI8, "300"), std::nullopt);

    const Literal likeU8{std::in_place_type<U8>, U8{0}};
    EXPECT_EQ(tryParseLiteral(likeU8, "-1"), std::nullopt);
  }

  TEST(LiteralText, RejectsAnEmptyDraft) {
    const Literal like{std::in_place_type<I32>, I32{0}};
    EXPECT_EQ(tryParseLiteral(like, ""), std::nullopt);
  }

  TEST(LiteralText, DoubleConstantsAreEditable) {
    const Literal like{std::in_place_type<F64>, 1.5};
    EXPECT_TRUE(isEditableLiteral(like));

    for (const auto& [text, expected] : {std::pair{"2.25", 2.25}, std::pair{"-3", -3.0}, std::pair{"1e3", 1000.0}}) {
      const std::optional<Literal> parsed = tryParseLiteral(like, text);
      ASSERT_TRUE(parsed.has_value()) << text;
      EXPECT_EQ(parsed->index(), like.index()) << text;
      EXPECT_EQ(std::get<F64>(*parsed), expected) << text;
    }
  }

  TEST(LiteralText, RejectsMalformedOrNonFiniteDouble) {
    const Literal like{std::in_place_type<F64>, 1.5};
    for (const char* text : {"", "abc", "1.5x", "1e999", "inf", "nan"}) {
      EXPECT_EQ(tryParseLiteral(like, text), std::nullopt) << text;
    }
  }

  TEST(LiteralText, RenderedDoublesParseBackExactly) {
    for (const F64 v : {1.5, 0.1}) {
      const Literal literal{std::in_place_type<F64>, v};
      EXPECT_EQ(tryParseLiteral(literal, renderLiteral(literal)), literal) << v;
    }
  }

  TEST(LiteralText, BoolConstantsAreNotEditable) {
    const Literal boolLiteral{std::in_place_type<BOOL>, true};

    EXPECT_FALSE(isEditableLiteral(boolLiteral));
    EXPECT_EQ(tryParseLiteral(boolLiteral, "true"), std::nullopt);
  }

}  // namespace
