#include "editor/core/literal_text.hpp"

#include <optional>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"

namespace {

  using namespace fluir::literals_types;
  using fluir::editor::isEditableLiteral;
  using fluir::editor::parseLiteralLike;
  using fluir::editor::renderLiteral;
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
    const std::optional<Literal> parsed = parseLiteralLike(like, "42");
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->index(), like.index());
    EXPECT_EQ(std::get<I16>(*parsed), 42);
  }

  TEST(LiteralText, RejectsTextThatIsNotANumber) {
    const Literal like{std::in_place_type<I32>, I32{0}};
    EXPECT_EQ(parseLiteralLike(like, "abc"), std::nullopt);
  }

  TEST(LiteralText, RejectsAValueOutsideTheAlternativesRange) {
    const Literal likeI8{std::in_place_type<I8>, I8{0}};
    EXPECT_EQ(parseLiteralLike(likeI8, "300"), std::nullopt);

    const Literal likeU8{std::in_place_type<U8>, U8{0}};
    EXPECT_EQ(parseLiteralLike(likeU8, "-1"), std::nullopt);
  }

  TEST(LiteralText, RejectsAnEmptyDraft) {
    const Literal like{std::in_place_type<I32>, I32{0}};
    EXPECT_EQ(parseLiteralLike(like, ""), std::nullopt);
  }

  TEST(LiteralText, DoubleAndBoolConstantsAreNotEditable) {
    const Literal doubleLiteral{std::in_place_type<F64>, 1.5};
    const Literal boolLiteral{std::in_place_type<BOOL>, true};

    EXPECT_FALSE(isEditableLiteral(doubleLiteral));
    EXPECT_FALSE(isEditableLiteral(boolLiteral));
    EXPECT_EQ(parseLiteralLike(doubleLiteral, "1.5"), std::nullopt);
    EXPECT_EQ(parseLiteralLike(boolLiteral, "true"), std::nullopt);
  }

}  // namespace
