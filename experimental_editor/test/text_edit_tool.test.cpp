#include "editor/tools/text_edit_tool.hpp"

#include <algorithm>
#include <memory>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/view/graph_draw.hpp"
#include "recording_renderer.hpp"
#include "tool_harness.hpp"

// Function 1 at the origin, body from y 25. Constant 10 (I32 42) at units (4,4)
// 10x5 -> {20,45,50,25}, text origin {24,49}, move grip {50,50,15,15}.
// Glyphs are 8 px, so a press 8 px past the origin puts the caret after one glyph.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  using fluir::ID;
  using fluir::editor::DeleteTransaction;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::InputEvent;
  using fluir::editor::nodeAt;
  using fluir::editor::Rect;
  using fluir::editor::TextEditTool;
  using fluir::editor::Vec2;
  using testutil::down;
  using testutil::key;
  using testutil::text;

  const EditorContext kCtx;
  const FullID kInt{1, 10};
  constexpr Vec2 kTextOrigin{24, 49};
  constexpr Vec2 kGrip{57, 57};
  constexpr Vec2 kFloatBody{24, 129};
  constexpr Vec2 kBinaryBody{154, 49};
  constexpr Vec2 kI8Body{24, 229};
  const Rect kIntRect{20, 45, 50, 25};

  fluir::pt::Constant constant(ID id, int y, fluir::pt::Literal value) {
    return {.id = id, .location = FlowGraphLocation{.x = 4, .y = y, .z = 1, .width = 10, .height = 5}, .value = value};
  }

  fluir::pt::ParseTree makeTree() {
    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.body.nodes.emplace(10, constant(10, 4, fluir::literals_types::I32{42}));
    fn.body.nodes.emplace(11, constant(11, 20, fluir::literals_types::F64{1.5}));
    fn.body.nodes.emplace(
      12,
      fluir::pt::Binary{
        .id = 12, .location = {.x = 30, .y = 4, .z = 1, .width = 10, .height = 5}, .op = fluir::Operator::PLUS});
    fn.body.nodes.emplace(13, constant(13, 40, fluir::literals_types::I8{42}));
    fluir::pt::ParseTree tree;
    tree.declarations.emplace(1, fluir::pt::Declaration{fn});
    return tree;
  }

  struct Harness {
    EditorState state{kCtx};
    TextEditTool tool;

    Harness() { state.editor.load(makeTree()); }

    bool send(const InputEvent& event) { return testutil::send(tool, state, event); }

    fluir::pt::Literal value(const FullID& path) const {
      return std::get<fluir::pt::Constant>(*nodeAt(state.editor.tree(), path)).value;
    }

    std::vector<testutil::DrawCall> draw() const {
      testutil::RecordingRenderer r;
      const auto boxes = fluir::editor::layoutGraph(state.editor.tree(), kCtx.layout);
      const fluir::editor::Subview root{state.view, Rect{0, 0, 800, 600}, r};
      fluir::editor::drawGraph(root, state.editor.tree(), boxes, std::nullopt, kCtx);
      tool.draw(root, state, boxes);
      return r.calls;
    }
  };

  // Index of the first Text call drawing `s` at `at`, or -1.
  long textIndex(const std::vector<testutil::DrawCall>& calls, std::string_view s, Vec2 at) {
    for (std::size_t i = 0; i < calls.size(); ++i) {
      if (calls[i].op == testutil::DrawCall::Op::Text && calls[i].text == s && calls[i].a == at) {
        return static_cast<long>(i);
      }
    }
    return -1;
  }

}  // namespace

TEST(TextEditTool, APressOnAnEditableConstantOpensThePrefilledFieldWithoutConsuming) {
  Harness h;

  EXPECT_FALSE(h.send(down(kTextOrigin)));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "42");
  EXPECT_EQ(h.tool.field()->caret(), 0u);
}

TEST(TextEditTool, APressPastTheTextOriginPlacesTheCaret) {
  Harness h;

  h.send(down(kTextOrigin + Vec2{8, 0}));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->caret(), 1u);
}

TEST(TextEditTool, RePressingTheOpenConstantReAimsTheCaretAndKeepsTheDraft) {
  Harness h;
  h.send(down(kTextOrigin));
  h.send(text("7"));

  h.send(down(kTextOrigin + Vec2{16, 0}));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "742");
  EXPECT_EQ(h.tool.field()->caret(), 2u);
}

TEST(TextEditTool, KeysAndTextAreIgnoredUntilAFieldIsOpen) {
  Harness h;

  EXPECT_FALSE(h.send(key(InputEvent::Key::Return)));
  EXPECT_FALSE(h.send(text("7")));
  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, NothingOpensOnNonEditableNodesGripsOrEmptySpace) {
  Harness h;

  for (const Vec2 at : {kBinaryBody, kGrip, Vec2{300, 300}, Vec2{-10, -10}}) {
    h.send(down(at));
    EXPECT_EQ(h.tool.field(), nullptr) << "press at (" << at.x << "," << at.y << ")";
  }
}

TEST(TextEditTool, ReturnAppliesOneUndoableEditAndCloses) {
  Harness h;
  h.send(down(kTextOrigin));
  ASSERT_TRUE(h.send(text("7")));
  EXPECT_EQ(h.value(kInt), fluir::pt::Literal{fluir::literals_types::I32{42}}) << "typing never touches the tree";

  EXPECT_TRUE(h.send(key(InputEvent::Key::Return)));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_EQ(h.value(kInt), fluir::pt::Literal{fluir::literals_types::I32{742}});
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.value(kInt), fluir::pt::Literal{fluir::literals_types::I32{42}});
}

TEST(TextEditTool, ARejectedDraftStaysOpenAndInvalid) {
  Harness h;
  h.send(down(kTextOrigin));
  h.send(text("x"));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Return)));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_TRUE(h.tool.field()->invalid());
  EXPECT_EQ(h.tool.field()->text(), "x42");
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(TextEditTool, ADraftPastTheTypesRangeIsRejected) {
  Harness h;
  h.send(down(kI8Body));
  h.send(key(InputEvent::Key::Home));
  h.send(text("9"));  // 942 overflows I8

  h.send(key(InputEvent::Key::Return));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_TRUE(h.tool.field()->invalid());
  EXPECT_EQ(h.value(FullID{1, 13}), fluir::pt::Literal{fluir::literals_types::I8{42}});
}

TEST(TextEditTool, AnF64ConstantEditsAndUndoes) {
  Harness h;
  const FullID path{1, 11};
  h.send(down(kFloatBody));
  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "1.5");

  h.send(text("2"));  // 21.5
  EXPECT_TRUE(h.send(key(InputEvent::Key::Return)));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_EQ(h.value(path), fluir::pt::Literal{fluir::literals_types::F64{21.5}});
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.value(path), fluir::pt::Literal{fluir::literals_types::F64{1.5}});
}

TEST(TextEditTool, AMalformedF64DraftStaysOpenAndInvalid) {
  Harness h;
  h.send(down(kFloatBody));
  h.send(text("."));  // .1.5

  h.send(key(InputEvent::Key::Return));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_TRUE(h.tool.field()->invalid());
  EXPECT_EQ(h.value(FullID{1, 11}), fluir::pt::Literal{fluir::literals_types::F64{1.5}});
}

TEST(TextEditTool, OtherKeysEditTheDraft) {
  Harness h;
  h.send(down(kTextOrigin));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Delete)));

  EXPECT_EQ(h.tool.field()->text(), "2");
}

TEST(TextEditTool, EscapeClosesWithoutAnEdit) {
  Harness h;
  h.send(down(kTextOrigin));
  h.send(text("7"));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Escape)));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(TextEditTool, APressElsewhereClosesWithoutAnEdit) {
  Harness h;
  h.send(down(kTextOrigin));
  h.send(text("7"));

  h.send(down(Vec2{300, 300}));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(TextEditTool, APressOnTheOpenConstantsGripClosesTheDraft) {
  Harness h;
  h.send(down(kTextOrigin));

  h.send(down(kGrip));

  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, CommittingTheUnchangedValueClosesWithoutAnEdit) {
  Harness h;
  h.send(down(kTextOrigin));

  h.send(key(InputEvent::Key::Return));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(TextEditTool, CancelClosesTheDraft) {
  Harness h;
  h.send(down(kTextOrigin));

  h.tool.cancel(h.state);

  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, TheDraftClosesWhenItsConstantVanishes) {
  Harness h;
  h.send(down(kTextOrigin));
  ASSERT_TRUE(h.state.editor.apply(std::make_unique<DeleteTransaction>(kInt)));

  EXPECT_FALSE(h.send(text("7")));

  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, TheDraftIsDrawnOverTheCommittedValue) {
  Harness h;
  h.send(down(kTextOrigin));
  h.send(text("7"));

  const auto calls = h.draw();

  const long value = textIndex(calls, "42", kTextOrigin);
  const long draft = textIndex(calls, "742", kTextOrigin);
  ASSERT_GE(draft, 0);
  EXPECT_GT(draft, value);
  // A fill of the node's rect between the two hides the committed value.
  const auto cover = std::find_if(calls.begin() + value + 1, calls.begin() + draft, [](const testutil::DrawCall& c) {
    return c.op == testutil::DrawCall::Op::Fill && c.rect == kIntRect;
  });
  EXPECT_NE(cover, calls.begin() + draft);
}

TEST(TextEditTool, AnInvalidDraftDrawsAnExtraErrorOutline) {
  Harness valid;
  Harness invalid;
  for (Harness* h : {&valid, &invalid}) {
    h->send(down(kTextOrigin));
    h->send(text("x"));
  }
  invalid.send(key(InputEvent::Key::Return));

  const auto outlines = [](const std::vector<testutil::DrawCall>& calls) {
    return std::count_if(calls.begin(), calls.end(), [](const testutil::DrawCall& c) {
      return c.op == testutil::DrawCall::Op::Rect && c.rect == kIntRect;
    });
  };
  EXPECT_GT(outlines(invalid.draw()), outlines(valid.draw()));
}
