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
#include "editor/actors/actor.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/actors/scene.hpp"
#include "editor/components/text_field.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/interaction.hpp"
#include "editor/core/viewport.hpp"
#include "editor/input.hpp"
#include "editor/transaction/transaction.hpp"
#include "recording_renderer.hpp"

// These tests drive ConstantActor through a real FocusInteraction, so they
// exercise the key policy the editor ships rather than a copy of it. What is
// under test here is the actor's own half: what prefills, and what commits.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::editor::Actor;
  using fluir::editor::ConstantActor;
  using fluir::editor::EditorContext;
  using fluir::editor::FocusInteraction;
  using fluir::editor::GraphScene;
  using fluir::editor::InputEvent;
  using fluir::editor::InteractionContext;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::TextField;
  using fluir::editor::Transaction;
  using fluir::editor::Vec2;
  using fluir::editor::Viewport;
  using testutil::DrawCall;
  using testutil::RecordingRenderer;
  using testutil::textStrings;

  /** Drives an actor through the real FocusInteraction. */
  class Keyboard {
   public:
    Keyboard(EditorContext& ctx, Actor& root) : ctx_(ctx), root_(root) { }

    /** Left-presses `parentLocalPos`; true when a draft is open afterwards. */
    bool press(Vec2 parentLocalPos) {
      send({.type = InputEvent::Type::MouseDown, .button = InputEvent::Button::Left, .pos = parentLocalPos});
      return root_.editor() != nullptr && root_.editor()->active();
    }

    bool key(InputEvent::Key key) { return send({.type = InputEvent::Type::KeyDown, .key = key}); }

    bool text(std::string utf8) { return send({.type = InputEvent::Type::TextInput, .text = std::move(utf8)}); }

    /** Drops focus the way a press elsewhere or a page reset would. */
    void blur() { focus_.reset(); }

   private:
    bool send(const InputEvent& event) {
      InteractionContext ictx{ctx_, view_, root_, Vec2{1000, 1000}};
      return focus_.onEvent(event, ictx);
    }

    EditorContext& ctx_;
    Actor& root_;
    Viewport view_;
    FocusInteraction focus_;
  };

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
  EditorContext ctx;
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));

  EXPECT_TRUE(actor.editor()->active());
  EXPECT_EQ(actor.editor()->field()->text(), "42");
  EXPECT_EQ(actor.editor()->field()->caret(), 0u);
}

TEST(ConstantEdit, APressPastTheTextOriginPlacesTheCaretThere) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kBodyPoint));  // 8px past the text origin
  EXPECT_EQ(actor.editor()->field()->caret(), 1u);

  // A press on an actor that already has focus only re-aims the caret.
  ASSERT_TRUE(kb.press(kTextOrigin));
  EXPECT_TRUE(actor.editor()->active());
  EXPECT_EQ(actor.editor()->field()->caret(), 0u);
  EXPECT_EQ(actor.editor()->field()->text(), "42");
}

TEST(ConstantEdit, KeysAndTextAreIgnoredUntilTheFieldIsOpen) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  EXPECT_FALSE(kb.key(InputEvent::Key::Return));
  EXPECT_FALSE(kb.text("7"));
  EXPECT_FALSE(actor.editor()->active());
}

TEST(ConstantEdit, TypingLeavesTheModelUntouchedUntilCommit) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));
  EXPECT_TRUE(kb.text("7"));

  EXPECT_EQ(actor.editor()->field()->text(), "742");
  EXPECT_EQ(std::get<fluir::literals_types::I32>(*actor.literal()), 42) << "the model only changes on commit";
  EXPECT_TRUE(edits.empty());
}

TEST(ConstantEdit, ReturnRaisesExactlyOneEditForTheWholeSession) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));
  kb.key(InputEvent::Key::End);
  kb.text("9");
  kb.text("0");
  kb.key(InputEvent::Key::Backspace);

  EXPECT_TRUE(kb.key(InputEvent::Key::Return));
  EXPECT_FALSE(actor.editor()->active());
  ASSERT_EQ(edits.size(), 1u);
  EXPECT_EQ(std::get<fluir::literals_types::I32>(applyToScene(ctx, *edits[0])), 429);
}

TEST(ConstantEdit, ARejectedCommitRaisesNoEditAndKeepsTheFieldOpen) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));
  kb.text("x");  // "x42" is not a number

  EXPECT_TRUE(kb.key(InputEvent::Key::Return));
  EXPECT_TRUE(actor.editor()->active()) << "a rejected draft stays open to be fixed";
  EXPECT_TRUE(actor.editor()->field()->invalid());
  EXPECT_EQ(actor.editor()->field()->text(), "x42");
  EXPECT_TRUE(edits.empty());
}

TEST(ConstantEdit, ADraftPastTheTypesRangeIsRejected) {
  ConstantActor actor(kFunctionId, makeConstant(fluir::literals_types::I8{42}), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));
  kb.key(InputEvent::Key::End);
  kb.text("9");  // 429 does not fit an I8

  kb.key(InputEvent::Key::Return);
  EXPECT_TRUE(actor.editor()->active());
  EXPECT_TRUE(actor.editor()->field()->invalid());
  EXPECT_TRUE(edits.empty());
  EXPECT_EQ(std::get<fluir::literals_types::I8>(*actor.literal()), 42);
}

TEST(ConstantEdit, EscapeRaisesNoEditAndRestoresTheRenderedValue) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));
  kb.text("7");
  EXPECT_TRUE(kb.key(InputEvent::Key::Escape));

  EXPECT_FALSE(actor.editor()->active());
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
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));
  kb.text("7");
  kb.blur();

  EXPECT_FALSE(actor.editor()->active());
  EXPECT_TRUE(edits.empty());
  EXPECT_EQ(std::get<fluir::literals_types::I32>(*actor.literal()), 42);
}

TEST(ConstantEdit, CommittingTheUnchangedValueRaisesNoEdit) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  std::vector<std::unique_ptr<Transaction>> edits;
  ctx.commit = sinkInto(edits);
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));
  kb.key(InputEvent::Key::Return);

  EXPECT_FALSE(actor.editor()->active()) << "an untouched draft closes cleanly";
  EXPECT_TRUE(edits.empty());
}

TEST(ConstantEdit, TheNodeDrawsTheDraftInsteadOfTheValueWhileEditing) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));
  kb.text("7");

  const std::vector<std::string> texts = textStrings(recordDraw(actor, ctx));
  EXPECT_NE(std::find(texts.begin(), texts.end(), "742"), texts.end());
  EXPECT_EQ(std::find(texts.begin(), texts.end(), "42"), texts.end()) << "the committed value is hidden";
}

TEST(ConstantEdit, AnInvalidDraftDrawsTheErrorAffordance) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  ASSERT_TRUE(kb.press(kTextOrigin));
  const std::size_t plain = outlinesOnBody(recordDraw(actor, ctx));

  kb.text("x");
  kb.key(InputEvent::Key::Return);
  ASSERT_TRUE(actor.editor()->field()->invalid());

  EXPECT_GT(outlinesOnBody(recordDraw(actor, ctx)), plain) << "an invalid draft adds an outline";
}

TEST(ConstantEdit, AFloatConstantClaimsNoFocus) {
  ConstantActor actor(kFunctionId, makeConstant(fluir::literals_types::F64{1.5}), Rect{0, 0, 25, 25});
  EditorContext ctx;
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  EXPECT_FALSE(kb.press(kBodyPoint));
  EXPECT_FALSE(actor.editor()->active());
}

TEST(ConstantEdit, ABoolConstantClaimsNoFocus) {
  ConstantActor actor(kFunctionId, makeConstant(fluir::literals_types::BOOL{true}), Rect{0, 0, 25, 25});
  EditorContext ctx;
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  EXPECT_FALSE(kb.press(kBodyPoint));
  EXPECT_FALSE(actor.editor()->active());
}

TEST(ConstantEdit, APressOnAGripClaimsNoFocus) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  EXPECT_FALSE(kb.press(kDragGripPoint));
  EXPECT_FALSE(kb.press(kResizeBarPoint));
  EXPECT_FALSE(actor.editor()->active());

  EXPECT_TRUE(kb.press(kBodyPoint)) << "the body still opens one";
}

// The grip predicate and the press read the same grip table: whatever claims a
// drag must also suppress the editor.
TEST(ConstantEdit, OnGripAgreesWithOnDragStart) {
  ConstantActor actor(kFunctionId, makeConstant(), Rect{0, 0, 25, 25});
  EditorContext ctx;
  actor.layout(ctx);
  Keyboard kb{ctx, actor};

  for (const Vec2 point : {kDragGripPoint, kResizeBarPoint, kBodyPoint, Vec2{100, 100}}) {
    EXPECT_EQ(actor.gestures()->onGrip(ctx, point), actor.gestures()->press(ctx, point)) << point.x << "," << point.y;
    actor.gestures()->cancel();
  }
}
