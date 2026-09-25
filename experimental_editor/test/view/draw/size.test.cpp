#include <optional>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/core/tree.hpp"
#include "editor/view/draw/binary.hpp"
#include "editor/view/draw/call.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/constant.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/draw/unary.hpp"
#include "editor/view/graph_layout.hpp"

// Each kind owns how it resizes and how small or large it may get.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::Limits;
  using fluir::editor::Part;
  using fluir::editor::Rect;
  using fluir::editor::Vec2i;
  namespace draw = fluir::editor::draw;
  namespace et = fluir::editor::et;

  const EditorContext kCtx;

  const et::Constant kI32{.id = 1, .location = {}, .value = fluir::literals_types::I32{0}};
  const et::Constant kBool{.id = 1, .location = {}, .value = fluir::literals_types::BOOL{true}};
  const et::Comment kComment{.id = 1, .location = {}, .text = ""};
  const et::Conditional kConditional{.id = 1,
                                     .location = {},
                                     .condition = {},
                                     .inputs = {},
                                     .outputs = {},
                                     .thenScope = xyz::indirect<et::Block>{},
                                     .elseScope = xyz::indirect<et::Block>{}};

  Rect atLeast(const Limits<Vec2i>& limits) {
    const double unit = kCtx.layout.unitPx;
    return {0, 0, limits.lower.x * unit, limits.lower.y * unit};
  }

}  // namespace

TEST(SizeFacts, OneRowNodesResizeAlongXOnly) {
  EXPECT_EQ(draw::resizePart(et::Binary{}), Part::ResizeX);
  EXPECT_EQ(draw::resizePart(et::Unary{}), Part::ResizeX);
  EXPECT_EQ(draw::resizePart(et::Call{}), Part::ResizeX);
  EXPECT_EQ(draw::resizePart(kI32), Part::ResizeX);
}

// A bool draws a fixed-size toggle square, so there is nothing to widen.
TEST(SizeFacts, ABoolConstantHasNoResizeGrip) { EXPECT_EQ(draw::resizePart(kBool), std::nullopt); }

TEST(SizeFacts, CommentsAndConditionalsResizeFromTheCorner) {
  EXPECT_EQ(draw::resizePart(kComment), Part::ResizeXY);
  EXPECT_EQ(draw::resizePart(kConditional), Part::ResizeXY);
}

TEST(SizeFacts, EveryLowerLimitIsWithinItsUpper) {
  for (const Limits<Vec2i>& limits : {draw::sizeLimits(et::Binary{}),
                                      draw::sizeLimits(et::Unary{}),
                                      draw::sizeLimits(et::Call{}),
                                      draw::sizeLimits(kI32),
                                      draw::sizeLimits(kComment),
                                      draw::sizeLimits(kConditional),
                                      draw::sizeLimits(et::FunctionDecl{})}) {
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
