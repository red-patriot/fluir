#include "editor/gesture/gesture_host.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/actor.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/gesture/grip.hpp"
#include "editor/gesture/inline_edit.hpp"
#include "editor/gesture/resize.hpp"
#include "editor/transaction/transaction.hpp"

// GestureHost owns the drag policy -- grip precedence, snapping, preview,
// commit and cancel -- as a member an actor holds, not a base it inherits. An
// actor supplies a grip table and its size limits; everything else is here.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::editor::Actor;
  using fluir::editor::actorBounds;
  using fluir::editor::Axes;
  using fluir::editor::dragGrip;
  using fluir::editor::EditorContext;
  using fluir::editor::GestureHost;
  using fluir::editor::Grip;
  using fluir::editor::horizResizeGrip;
  using fluir::editor::InlineEdit;
  using fluir::editor::Limits;
  using fluir::editor::Rect;
  using fluir::editor::ResizeGesture;
  using fluir::editor::TextField;
  using fluir::editor::Transaction;
  using fluir::editor::Vec2;
  using fluir::editor::Vec2i;

  constexpr fluir::ID kFunctionId = 100;
  constexpr fluir::ID kNodeId = 1;

  constexpr Limits<Vec2i> kSize{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};

  // width/height = 5 units at the default unitPx of 5 -> a 25x25 rect, so the
  // drag grip is {5, 5, 15, 15} and the resize bar {20, 0, 5, 25}.
  const FlowGraphLocation kLoc{.x = 0, .y = 0, .z = 0, .width = 5, .height = 5};

  constexpr Vec2 kDragGrip{10, 10};
  constexpr Vec2 kResizeBar{22, 22};
  constexpr Vec2 kBody{12, 22};

  /** A bar across the bottom edge: the stand-in for a gesture added later. */
  Grip vertResizeGrip() {
    return Grip{
      .rect =
        [](const FlowGraphLocation& loc) { return Rect{0, loc.height - 1.0, static_cast<double>(loc.width), 1.0}; },
      .begin = [] { return std::make_unique<ResizeGesture>(Axes::XY, kSize); },
      .frame = actorBounds};
  }

  /** A node-shaped actor: a grip table, size limits, and an editable body. */
  class StubActor : public Actor {
   public:
    explicit StubActor(std::vector<Grip> grips) :
      Actor(Rect{0, 0, 25, 25}), location_(kLoc), gestures_(*this, std::move(grips), kSize) { }

    using Actor::location;
    FlowGraphLocation* location() override { return &location_; }
    std::optional<fluir::FullID> selectionId() const override { return fluir::FullID{kFunctionId, kNodeId}; }

    GestureHost* gestures() override { return &gestures_; }
    InlineEdit* editor() override { return &edit_; }

    FlowGraphLocation preview() const { return gestures_.preview(location_); }

   private:
    FlowGraphLocation location_;
    GestureHost gestures_;
    InlineEdit edit_{[] { return std::optional<std::string>{"42"}; },
                     [](const EditorContext&, const std::string&) { return true; }};
  };

  /** The grips a node carries today. */
  std::vector<Grip> nodeGrips() { return {dragGrip(), horizResizeGrip(kSize)}; }

  EditorContext::TransactionSink sinkInto(std::vector<std::unique_ptr<Transaction>>& edits) {
    return [&edits](std::unique_ptr<Transaction> edit) {
      edits.push_back(std::move(edit));
      return true;
    };
  }

}  // namespace

TEST(GestureHost, APressOffEveryGripStartsNoGesture) {
  StubActor actor(nodeGrips());
  const EditorContext ctx;

  EXPECT_FALSE(actor.gestures()->press(ctx, kBody));
  EXPECT_FALSE(actor.gestures()->press(ctx, Vec2{100, 100}));
  EXPECT_FALSE(actor.gestures()->active());
}

TEST(GestureHost, TheFirstGripListedWinsWhereTwoOverlap) {
  StubActor actor(nodeGrips());
  const EditorContext ctx;

  // {19, 10} is the drag grip's rightmost column, inside the resize bar's row.
  ASSERT_TRUE(actor.gestures()->press(ctx, Vec2{19, 10}));
  actor.gestures()->drag(ctx, Vec2{10, 0});

  EXPECT_EQ(actor.preview().x, 2) << "the drag grip claimed it, so the node moved";
  EXPECT_EQ(actor.preview().width, 5) << "and nothing resized";
}

TEST(GestureHost, ThePreviewDrivesLayoutUntilTheGestureCommits) {
  StubActor actor(nodeGrips());
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);

  ASSERT_TRUE(actor.gestures()->press(ctx, kDragGrip));
  actor.gestures()->drag(ctx, Vec2{10, 0});

  EXPECT_EQ(actor.preview().x, 2);
  EXPECT_EQ(actor.location()->x, 0) << "the model is untouched until release";
  EXPECT_TRUE(edits.empty());
}

TEST(GestureHost, ReleaseCommitsAndDropsTheGesture) {
  StubActor actor(nodeGrips());
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);

  ASSERT_TRUE(actor.gestures()->press(ctx, kDragGrip));
  actor.gestures()->drag(ctx, Vec2{10, 0});
  actor.gestures()->release(ctx);

  EXPECT_EQ(edits.size(), 1u);
  EXPECT_FALSE(actor.gestures()->active());
  EXPECT_EQ(actor.preview().x, 0) << "the preview is gone with the gesture";
}

TEST(GestureHost, CancelDropsThePreviewAndRaisesNothing) {
  StubActor actor(nodeGrips());
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);

  ASSERT_TRUE(actor.gestures()->press(ctx, kDragGrip));
  actor.gestures()->drag(ctx, Vec2{10, 0});
  actor.gestures()->cancel();

  EXPECT_EQ(actor.preview().x, 0);
  EXPECT_TRUE(edits.empty());
}

// Layout must never show a size the actor is not allowed to be, even when the
// model carries one -- the same limits the resize gesture is bounded by.
TEST(GestureHost, SizeLimitsClampTheLocationEvenWithNoGesture) {
  StubActor actor(nodeGrips());
  actor.location()->width = 1;

  EXPECT_EQ(actor.preview().width, 4);
}

TEST(GestureHost, TheGripPredicateAgreesWithWhatClaimsADrag) {
  StubActor actor(nodeGrips());
  const EditorContext ctx;

  for (const Vec2 point : {kDragGrip, kResizeBar, kBody, Vec2{100, 100}}) {
    EXPECT_EQ(actor.gestures()->onGrip(ctx, point), actor.gestures()->press(ctx, point)) << point.x << "," << point.y;
    actor.gestures()->cancel();
  }
}

// The point of the grip table: a new gesture costs one row and nothing else.
TEST(GestureHost, AddingAGripIsTheWholeCostOfAddingAGesture) {
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  const Vec2 bottomBar{5, 22};

  StubActor without(nodeGrips());
  ASSERT_FALSE(without.gestures()->press(ctx, bottomBar)) << "no grip lives there yet";

  std::vector<Grip> grips = nodeGrips();
  grips.push_back(vertResizeGrip());
  StubActor with(std::move(grips));

  ASSERT_TRUE(with.gestures()->press(ctx, bottomBar));
  with.gestures()->drag(ctx, Vec2{0, 10});
  EXPECT_EQ(with.preview().height, 7) << "preview, snapping and commit came for free";

  with.gestures()->release(ctx);
  EXPECT_EQ(edits.size(), 1u);
}
