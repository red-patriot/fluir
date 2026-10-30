#include <optional>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/view/draw/binary.hpp"
#include "editor/view/draw/call.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/constant.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/draw/unary.hpp"

// Each kind owns how it resizes and how small or large it may get.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::Limits;
  using fluir::editor::Rect;
  using fluir::editor::ResizeAxes;
  using fluir::editor::Vec2i;
  namespace draw = fluir::editor::draw;
  namespace pt = fluir::pt;

  const EditorContext kCtx;

  const pt::Constant kI32{.id = 1, .location = {}, .value = fluir::literals_types::I32{0}};
  const pt::Constant kBool{.id = 1, .location = {}, .value = fluir::literals_types::BOOL{true}};
  const pt::Comment kComment{.id = 1, .location = {}, .text = ""};
  const pt::Conditional kConditional{.id = 1,
                                     .location = {},
                                     .condition = {},
                                     .inputs = {},
                                     .outputs = {},
                                     .thenScope = xyz::indirect<pt::Block>{},
                                     .elseScope = xyz::indirect<pt::Block>{}};

  Rect atLeast(const Limits<Vec2i>& limits) {
    const double unit = kCtx.layout.unitPx;
    return {0, 0, limits.lower.x * unit, limits.lower.y * unit};
  }

}  // namespace

TEST(SizeFacts, OneRowNodesResizeAlongXOnly) {
  EXPECT_EQ(draw::resizePart(pt::Binary{}), ResizeAxes::X);
  EXPECT_EQ(draw::resizePart(pt::Unary{}), ResizeAxes::X);
  EXPECT_EQ(draw::resizePart(pt::Call{}), ResizeAxes::X);
  EXPECT_EQ(draw::resizePart(kI32), ResizeAxes::X);
}

// A bool draws a fixed-size toggle square, so there is nothing to widen.
TEST(SizeFacts, ABoolConstantHasNoResizeGrip) { EXPECT_EQ(draw::resizePart(kBool), std::nullopt); }

TEST(SizeFacts, CommentsAndConditionalsResizeFromTheCorner) {
  EXPECT_EQ(draw::resizePart(kComment), ResizeAxes::XY);
  EXPECT_EQ(draw::resizePart(kConditional), ResizeAxes::XY);
}

TEST(SizeFacts, EveryLowerLimitIsWithinItsUpper) {
  for (const Limits<Vec2i>& limits : {draw::sizeLimits(pt::Binary{}),
                                      draw::sizeLimits(pt::Unary{}),
                                      draw::sizeLimits(pt::Call{}),
                                      draw::sizeLimits(kI32),
                                      draw::sizeLimits(kComment),
                                      draw::sizeLimits(kConditional),
                                      draw::sizeLimits(pt::FunctionDecl{})}) {
    EXPECT_LE(limits.lower.x, limits.upper.x);
    EXPECT_LE(limits.lower.y, limits.upper.y);
  }
}

TEST(SizeFacts, AMinimalCommentKeepsItsCornerGripClearOfItsMoveGrip) {
  const Rect rect = atLeast(draw::sizeLimits(kComment));
  const double unit = kCtx.layout.unitPx;

  EXPECT_FALSE(fluir::editor::intersect(fluir::editor::resizeCorner(rect, unit), fluir::editor::moveGrip(rect, unit)));
}

TEST(SizeFacts, AMinimalConditionalKeepsRoomUnderItsHeader) {
  EXPECT_GT(atLeast(draw::sizeLimits(kConditional)).h, kCtx.layout.headerH());
}
