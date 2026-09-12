#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/literal_types.hpp"
#include "compiler/models/location.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/actors/scene.hpp"
#include "editor/components/text_field.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "editor/transaction/transaction.hpp"
#include "recording_renderer.hpp"

// These tests drive ConstantActor through the focus protocol it implements --
// onFocus / onTextInput / onKey / onBlur -- exactly as FocusInteraction does.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::editor::ConstantActor;
  using fluir::editor::EditorContext;
  using fluir::editor::GraphScene;
  using fluir::editor::InputEvent;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::TextField;
  using fluir::editor::Transaction;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::DrawCall;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

  constexpr ID kFunctionId = 100;
  constexpr ID kNodeId = 1;

  // width/height = 5 units, unitPx = 5 (EditorContext default) -> a 25x25 node
  // rect: the drag grip is {5, 5, 15, 15} and the resize bar {20, 0, 5, 25}.
  const FlowGraphLocation kNodeLoc{.x = 0, .y = 0, .z = 0, .width = 5, .height = 5};

  // On the text origin (bounds().x + textPad), so the caret lands at index 0.
  constexpr Vec2 kTextOrigin{4, 22};
  constexpr Vec2 kBodyPoint{12, 22};  ///< inside the node, clear of both grips
  constexpr Vec2 kDragGripPoint{10, 10};
  constexpr Vec2 kResizeBarPoint{22, 22};

  fluir::pt::Constant makeConstant(fluir::pt::Literal value = fluir::literals_types::I32{42}) {
    fluir::pt::Constant constant;
    constant.id = kNodeId;
    constant.location = kNodeLoc;
    constant.value = std::move(value);
    return constant;
  }

  // A scene holding exactly the node the bare actor stands for, so an edit the
  // actor raises can be applied and read back.
  fluir::pt::ParseTree makeTree(fluir::pt::Literal value = fluir::literals_types::I32{42}) {
    fluir::pt::FunctionDecl fn;
    fn.id = kFunctionId;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(kNodeId, makeConstant(std::move(value)));

    fluir::pt::ParseTree tree;
    tree.declarations.emplace(fn.id, fluir::pt::Declaration{fn});
    return tree;
  }

  /** Records a sink into `edits`, the way a live ModulePage would. */
  EditorContext::TransactionSink sinkInto(std::vector<std::unique_ptr<Transaction>>& edits) {
    return [&edits](std::unique_ptr<Transaction> edit) {
      edits.push_back(std::move(edit));
      return true;
    };
  }

  /** Applies `edit` to a scene of one constant and returns the literal it left. */
  fluir::pt::Literal applyToScene(const EditorContext& ctx, Transaction& edit) {
    GraphScene scene;
    scene.build(ctx, makeTree());
    EXPECT_TRUE(edit.execute(scene));
    auto* node = dynamic_cast<ConstantActor*>(scene.find(kFunctionId, kNodeId));
    EXPECT_NE(node, nullptr);
    return *node->literal();
  }

  // Identity Viewport + a root Subview at world origin, so
  // `body.toScreen(local) == local`; returns every recorded draw call.
  std::vector<DrawCall> recordDraw(ConstantActor& actor, const EditorContext& ctx) {
    actor.layout(ctx);
    RecordingRenderer renderer;
    const Viewport viewport;
    {
      const Subview body{viewport, Rect{0, 0, 1000, 1000}, renderer};
      actor.draw(body, ctx);
    }
    return renderer.calls;
  }

  /** How many outlines were stroked around the whole node rect. */
  std::size_t outlinesOnBody(const std::vector<DrawCall>& calls) {
    std::size_t count = 0;
    for (const Rect& r : testutil::rectsOf(calls)) {
      if (r == Rect{0, 0, 25, 25}) {
        ++count;
      }
    }
    return count;
  }

}  // namespace

TEST(ConstantEdit, APressOnTheBodyOpensTheFieldPrefilledWithTheCurrentValue) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  const EditorContext ctx;
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));

  EXPECT_TRUE(actor.field().active());
  EXPECT_EQ(actor.field().text(), "42");
  EXPECT_EQ(actor.field().caret(), 0u);
}

TEST(ConstantEdit, APressPastTheTextOriginPlacesTheCaretThere) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  const EditorContext ctx;
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kBodyPoint));  // 8px past the text origin
  EXPECT_EQ(actor.field().caret(), 1u);

  // A press on an actor that already has focus only re-aims the caret.
  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  EXPECT_TRUE(actor.field().active());
  EXPECT_EQ(actor.field().caret(), 0u);
  EXPECT_EQ(actor.field().text(), "42");
}

TEST(ConstantEdit, KeysAndTextAreIgnoredUntilTheFieldIsOpen) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  const EditorContext ctx;
  actor.layout(ctx);

  EXPECT_FALSE(actor.onKey(ctx, InputEvent::Key::Return));
  EXPECT_FALSE(actor.onTextInput(ctx, "7"));
  EXPECT_FALSE(actor.field().active());
}

TEST(ConstantEdit, TypingLeavesTheModelUntouchedUntilCommit) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  EXPECT_TRUE(actor.onTextInput(ctx, "7"));

  EXPECT_EQ(actor.field().text(), "742");
  EXPECT_EQ(std::get<fluir::literals_types::I32>(*actor.literal()), 42) << "the model only changes on commit";
  EXPECT_TRUE(edits.empty());
}

TEST(ConstantEdit, ReturnRaisesExactlyOneEditForTheWholeSession) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  actor.onKey(ctx, InputEvent::Key::End);
  actor.onTextInput(ctx, "9");
  actor.onTextInput(ctx, "0");
  actor.onKey(ctx, InputEvent::Key::Backspace);

  EXPECT_TRUE(actor.onKey(ctx, InputEvent::Key::Return));
  EXPECT_FALSE(actor.field().active());
  ASSERT_EQ(edits.size(), 1u);
  EXPECT_EQ(std::get<fluir::literals_types::I32>(applyToScene(ctx, *edits[0])), 429);
}

TEST(ConstantEdit, ARejectedCommitRaisesNoEditAndKeepsTheFieldOpen) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  actor.onTextInput(ctx, "x");  // "x42" is not a number

  EXPECT_TRUE(actor.onKey(ctx, InputEvent::Key::Return));
  EXPECT_TRUE(actor.field().active()) << "a rejected draft stays open to be fixed";
  EXPECT_TRUE(actor.field().invalid());
  EXPECT_EQ(actor.field().text(), "x42");
  EXPECT_TRUE(edits.empty());
}

TEST(ConstantEdit, ADraftPastTheTypesRangeIsRejected) {
  ConstantActor actor(kFunctionId, makeConstant(fluir::literals_types::I8{42}), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  actor.onKey(ctx, InputEvent::Key::End);
  actor.onTextInput(ctx, "9");  // 429 does not fit an I8

  actor.onKey(ctx, InputEvent::Key::Return);
  EXPECT_TRUE(actor.field().active());
  EXPECT_TRUE(actor.field().invalid());
  EXPECT_TRUE(edits.empty());
  EXPECT_EQ(std::get<fluir::literals_types::I8>(*actor.literal()), 42);
}

TEST(ConstantEdit, EscapeRaisesNoEditAndRestoresTheRenderedValue) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  actor.onTextInput(ctx, "7");
  EXPECT_TRUE(actor.onKey(ctx, InputEvent::Key::Escape));

  EXPECT_FALSE(actor.field().active());
  EXPECT_TRUE(edits.empty());
  EXPECT_EQ(std::get<fluir::literals_types::I32>(*actor.literal()), 42);

  const std::vector<std::string> texts = textStrings(recordDraw(actor, ctx));
  EXPECT_NE(std::find(texts.begin(), texts.end(), "42"), texts.end());
  EXPECT_EQ(std::find(texts.begin(), texts.end(), "742"), texts.end());
}

TEST(ConstantEdit, BlurDropsTheDraftWithoutRaisingAnEdit) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  actor.onTextInput(ctx, "7");
  actor.onBlur();

  EXPECT_FALSE(actor.field().active());
  EXPECT_TRUE(edits.empty());
  EXPECT_EQ(std::get<fluir::literals_types::I32>(*actor.literal()), 42);
}

TEST(ConstantEdit, CommittingTheUnchangedValueRaisesNoEdit) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  actor.onKey(ctx, InputEvent::Key::Return);

  EXPECT_FALSE(actor.field().active()) << "an untouched draft closes cleanly";
  EXPECT_FALSE(actor.field().invalid());
  EXPECT_TRUE(edits.empty());
}

TEST(ConstantEdit, TheNodeDrawsTheDraftInsteadOfTheValueWhileEditing) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  const EditorContext ctx;
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  actor.onTextInput(ctx, "7");

  const std::vector<std::string> texts = textStrings(recordDraw(actor, ctx));
  EXPECT_NE(std::find(texts.begin(), texts.end(), "742"), texts.end());
  EXPECT_EQ(std::find(texts.begin(), texts.end(), "42"), texts.end()) << "the committed value is hidden";
}

TEST(ConstantEdit, AnInvalidDraftDrawsTheErrorAffordance) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  actor.layout(ctx);

  ASSERT_TRUE(actor.onFocus(ctx, kTextOrigin));
  const std::size_t plain = outlinesOnBody(recordDraw(actor, ctx));

  actor.onTextInput(ctx, "x");
  actor.onKey(ctx, InputEvent::Key::Return);
  ASSERT_TRUE(actor.field().invalid());

  EXPECT_GT(outlinesOnBody(recordDraw(actor, ctx)), plain) << "an invalid draft adds an outline";
}

TEST(ConstantEdit, AFloatConstantClaimsNoFocus) {
  ConstantActor actor(kFunctionId, makeConstant(fluir::literals_types::F64{1.5}), Rect{0, 0, 25, 25});
  const EditorContext ctx;
  actor.layout(ctx);

  EXPECT_FALSE(actor.onFocus(ctx, kBodyPoint));
  EXPECT_FALSE(actor.field().active());
}

TEST(ConstantEdit, ABoolConstantClaimsNoFocus) {
  ConstantActor actor(kFunctionId, makeConstant(fluir::literals_types::BOOL{true}), Rect{0, 0, 25, 25});
  const EditorContext ctx;
  actor.layout(ctx);

  EXPECT_FALSE(actor.onFocus(ctx, kBodyPoint));
  EXPECT_FALSE(actor.field().active());
}

TEST(ConstantEdit, APressOnAGripClaimsNoFocus) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  const EditorContext ctx;
  actor.layout(ctx);

  EXPECT_FALSE(actor.onFocus(ctx, kDragGripPoint));
  EXPECT_FALSE(actor.onFocus(ctx, kResizeBarPoint));
  EXPECT_FALSE(actor.field().active());

  EXPECT_TRUE(actor.onFocus(ctx, kBodyPoint)) << "the body still opens one";
}

// The grip predicate and `onDragStart` read the same grips: whatever claims a
// drag must also suppress the editor.
TEST(ConstantEdit, OnHandlesAgreesWithOnDragStart) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  const EditorContext ctx;
  actor.layout(ctx);

  for (const Vec2 point : {kDragGripPoint, kResizeBarPoint, kBodyPoint, Vec2{100, 100}}) {
    EXPECT_EQ(actor.onHandles(ctx, point), actor.onDragStart(ctx, point)) << point.x << "," << point.y;
    actor.onDragCancel();
  }
}
