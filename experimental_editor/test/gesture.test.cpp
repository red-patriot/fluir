#include "editor/gesture/gesture.hpp"

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/scene.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/gesture/move.hpp"
#include "editor/gesture/resize.hpp"
#include "editor/transaction/transaction.hpp"

// A gesture exists only while it runs: it accumulates whole grid units, shows
// them as a preview, and raises at most one edit when it commits.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::editor::Axes;
  using fluir::editor::EditorContext;
  using fluir::editor::GraphScene;
  using fluir::editor::GridAccumulator;
  using fluir::editor::Limits;
  using fluir::editor::MoveGesture;
  using fluir::editor::ResizeGesture;
  using fluir::editor::Transaction;
  using fluir::editor::Vec2;
  using fluir::editor::Vec2i;

  const fluir::FullID kId{100, 1};
  const FlowGraphLocation kLoc{.x = 2, .y = 3, .z = 0, .width = 5, .height = 5};

  constexpr Limits<Vec2i> kWide{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};

  EditorContext::TransactionSink sinkInto(std::vector<std::unique_ptr<Transaction>>& edits) {
    return [&edits](std::unique_ptr<Transaction> edit) {
      edits.push_back(std::move(edit));
      return true;
    };
  }

}  // namespace

TEST(GridAccumulator, SubUnitDeltasAccumulateUntilTheyCrossAUnit) {
  GridAccumulator acc;

  EXPECT_EQ(acc.fold(5.0, Vec2{3, 0}), (Vec2i{0, 0})) << "3px is less than one 5px unit";
  EXPECT_EQ(acc.fold(5.0, Vec2{3, 0}), (Vec2i{1, 0})) << "6px total crosses one unit";
}

TEST(GridAccumulator, TheRemainderIsCarriedNotDropped) {
  GridAccumulator acc;

  acc.fold(5.0, Vec2{7, 0});  // one unit, 2px carried
  EXPECT_EQ(acc.fold(5.0, Vec2{3, 0}), (Vec2i{1, 0})) << "the carried 2px completes the next unit";
}

TEST(GridAccumulator, NegativeDeltasStepTheOtherWay) {
  GridAccumulator acc;

  EXPECT_EQ(acc.fold(5.0, Vec2{-12, -5}), (Vec2i{-2, -1}));
}

TEST(MoveGesture, PreviewShiftsByWholeUnitsOnly) {
  MoveGesture gesture;
  const EditorContext ctx;  // unitPx = 5

  gesture.update(ctx, Vec2{12, 3});

  EXPECT_EQ(gesture.preview(kLoc).x, 4) << "12px is 2 units past x = 2";
  EXPECT_EQ(gesture.preview(kLoc).y, 3) << "3px has not crossed a unit yet";
}

TEST(MoveGesture, CommitRaisesOneEditForTheWholeGesture) {
  MoveGesture gesture;
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);

  gesture.update(ctx, Vec2{5, 0});
  gesture.update(ctx, Vec2{5, 5});
  gesture.commit(ctx, kLoc, kId);

  EXPECT_EQ(edits.size(), 1u);
}

TEST(MoveGesture, AGestureThatMovedNothingRaisesNoEdit) {
  MoveGesture gesture;
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);

  gesture.update(ctx, Vec2{3, 3});  // sub-unit
  gesture.commit(ctx, kLoc, kId);

  EXPECT_TRUE(edits.empty());
}

TEST(ResizeGesture, TheXAxisLeavesHeightAlone) {
  ResizeGesture gesture(Axes::X, kWide);
  const EditorContext ctx;

  gesture.update(ctx, Vec2{10, 10});

  EXPECT_EQ(gesture.preview(kLoc).width, 7);
  EXPECT_EQ(gesture.preview(kLoc).height, 5) << "an x-axis resize never touches height";
}

TEST(ResizeGesture, TheXYAxesDriveBothDimensions) {
  ResizeGesture gesture(Axes::XY, kWide);
  const EditorContext ctx;

  gesture.update(ctx, Vec2{10, 10});

  EXPECT_EQ(gesture.preview(kLoc).width, 7);
  EXPECT_EQ(gesture.preview(kLoc).height, 7);
}

TEST(ResizeGesture, ThePreviewClampsToTheLimits) {
  ResizeGesture gesture(Axes::X, kWide);
  const EditorContext ctx;

  gesture.update(ctx, Vec2{-100, 0});  // -20 units, far past the minimum

  EXPECT_EQ(gesture.preview(kLoc).width, 4);
}

TEST(ResizeGesture, AGestureClampedBackToTheCurrentSizeRaisesNoEdit) {
  ResizeGesture gesture(Axes::X, Limits<Vec2i>{.lower = Vec2i{5, 0}, .upper = Vec2i{1000, 1000}});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);

  gesture.update(ctx, Vec2{-100, 0});  // clamps straight back to width 5
  gesture.commit(ctx, kLoc, kId);

  EXPECT_TRUE(edits.empty());
}
