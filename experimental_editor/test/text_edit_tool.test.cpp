#include "editor/tools/text_edit_tool.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
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
// Function "f" header {0,0,500,25}, text origin {4,4}. Params x, y: rails {0,25,75,25}, {0,50,75,25}; return rail
// {475,25,25,25}. Call 14 ("g", arguments a, b) at units (30,20) 10x15 -> {150,125,50,75}, label row
// {150,125,50,25}, text origin {154,129}, argument rows from y 150 and 175.
// Top-level comment 20 ("hi there") at units (40,40) 20x10 -> {200,200,100,50}, text rect {204,204,92,42}. In-body
// comment 16 ("inner") at units (60,4) 20x10 -> {300,45,100,50}, text rect {304,49,92,42}. The recorder lays wrapped
// text out in 8 px cells.

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
  // Value text origins sit after each constant's small type label (textPad + glyphs at 0.8x).
  constexpr Vec2 kTextOrigin{47.2, 49};
  constexpr Vec2 kIntType{24, 49};
  constexpr Vec2 kGrip{57, 57};
  constexpr Vec2 kFloatBody{50, 129};
  constexpr Vec2 kBinaryBody{154, 49};
  constexpr Vec2 kI8Body{40, 229};
  const Rect kIntRect{43.2, 45, 26.8, 25};
  const FullID kFn{1};
  const FullID kCall{1, 14};
  constexpr Vec2 kFnName{20.8, 4};
  constexpr Vec2 kFnKeyword{4, 4};
  constexpr Vec2 kCallLabel{154, 129};
  constexpr Vec2 kCallArgRow{154, 154};
  constexpr Vec2 kCallArgRow1{154, 179};
  // Param y's name region, clear of constant 10; type "i32" spans textPad + 3 glyphs at 0.8x.
  constexpr Vec2 kParamRail1{72, 54};
  constexpr Vec2 kParamRail1Text{27.2, 54};
  constexpr Vec2 kParamRail1Type{4, 54};
  constexpr Vec2 kReturnRail{479, 29};
  const Rect kParamRail1Rect{23.2, 50, 51.8, 25};
  const Rect kCallArgRow1Rect{150, 175, 50, 25};
  const Rect kHeaderRect{16.8, 0, 483.2, 25};
  const Rect kCallLabelRect{150, 125, 50, 25};
  const FullID kComment{20};
  const FullID kInnerComment{1, 16};
  const Rect kCommentRect{200, 200, 100, 50};
  const Rect kCommentTextRect{204, 204, 92, 42};
  constexpr Vec2 kCommentText{220, 205};  // cell column 2 of row 0
  constexpr Vec2 kInnerCommentText{304, 49};

  fluir::pt::Constant constant(ID id, int y, fluir::pt::Literal value) {
    return {.id = id, .location = FlowGraphLocation{.x = 4, .y = y, .z = 1, .width = 10, .height = 5}, .value = value};
  }

  fluir::pt::ParseTree makeTree() {
    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.input =
      fluir::pt::FunctionDecl::InputBlock{.parameters = {{.id = 2, .index = 0, .name = "x", .typeName = "i32"},
                                                         {.id = 3, .index = 1, .name = "y", .typeName = "i32"}}};
    fn.output =
      fluir::pt::FunctionDecl::OutputBlock{.ret = fluir::pt::FunctionDecl::Return{.id = 4, .typeName = "i32"}};
    fn.body.nodes.emplace(10, constant(10, 4, fluir::literals_types::I32{42}));
    fn.body.nodes.emplace(11, constant(11, 20, fluir::literals_types::F64{1.5}));
    fn.body.nodes.emplace(
      12,
      fluir::pt::Binary{
        .id = 12, .location = {.x = 30, .y = 4, .z = 1, .width = 10, .height = 5}, .op = fluir::Operator::PLUS});
    fn.body.nodes.emplace(13, constant(13, 40, fluir::literals_types::I8{42}));
    fn.body.nodes.emplace(14,
                          fluir::pt::Call{.id = 14,
                                          .location = {.x = 30, .y = 20, .z = 1, .width = 10, .height = 15},
                                          .target = "g",
                                          .arguments = {{.name = "a", .index = 0}, {.name = "b", .index = 1}}});
    fn.body.nodes.emplace(
      16,
      fluir::pt::Comment{.id = 16, .location = {.x = 60, .y = 4, .z = 1, .width = 20, .height = 10}, .text = "inner"});
    fluir::pt::ParseTree tree;
    tree.declarations.emplace(1, fluir::pt::Declaration{fn});
    tree.declarations.emplace(
      20,
      fluir::pt::Declaration{fluir::pt::Comment{
        .id = 20, .location = {.x = 40, .y = 40, .z = 2, .width = 20, .height = 10}, .text = "hi there"}});
    return tree;
  }

  struct Harness {
    EditorState state{kCtx};
    TextEditTool tool;
    mutable testutil::RecordingRenderer recorder;

    Harness() {
      state.editor.load(makeTree());
      state.text = &recorder;
    }

    bool send(const InputEvent& event) { return testutil::send(tool, state, event); }

    fluir::pt::Literal value(const FullID& path) const {
      return std::get<fluir::pt::Constant>(*nodeAt(state.editor.tree(), path)).value;
    }

    const std::string& fnName() const { return fluir::editor::functionAt(state.editor.tree(), kFn)->name; }

    const std::string& callTarget() const {
      return std::get<fluir::pt::Call>(*nodeAt(state.editor.tree(), kCall)).target;
    }

    // Vector position equals `index` in this tree.
    const std::string& paramName(std::size_t index) const {
      return fluir::editor::functionAt(state.editor.tree(), kFn)->input->parameters[index].name;
    }

    const std::string& argName(std::size_t index) const {
      return std::get<fluir::pt::Call>(*nodeAt(state.editor.tree(), kCall)).arguments[index].name;
    }

    const std::string& commentText(const FullID& path) const {
      if (path.size() == 1) {
        return std::get<fluir::pt::Comment>(state.editor.tree().declarations.at(path[0])).text;
      }
      return std::get<fluir::pt::Comment>(*nodeAt(state.editor.tree(), path)).text;
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
      if (calls[i].op == testutil::DrawCall::Op::Text && calls[i].text == s && std::abs(calls[i].a.x - at.x) <= 1e-6 &&
          std::abs(calls[i].a.y - at.y) <= 1e-6) {
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

TEST(TextEditTool, APressOnAConstantsTypeOpensNothing) {
  Harness h;

  h.send(down(kIntType));

  EXPECT_EQ(h.tool.field(), nullptr);
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
    return c.op == testutil::DrawCall::Op::Fill && testutil::detail::rectNear(c.rect, kIntRect, 1e-6);
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
      return c.op == testutil::DrawCall::Op::Rect && testutil::detail::rectNear(c.rect, kIntRect, 1e-6);
    });
  };
  EXPECT_GT(outlines(invalid.draw()), outlines(valid.draw()));
}

TEST(TextEditTool, APressOnAFunctionHeaderOpensItsName) {
  Harness h;

  EXPECT_FALSE(h.send(down(kFnName)));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "f");
}

TEST(TextEditTool, APressOnTheFnKeywordOpensNothing) {
  Harness h;

  h.send(down(kFnKeyword));

  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, ReturnRenamesTheFunctionUndoably) {
  Harness h;
  h.send(down(kFnName));
  h.send(key(InputEvent::Key::End));
  h.send(text("oo"));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Return)));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_EQ(h.fnName(), "foo");
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.fnName(), "f");
}

TEST(TextEditTool, AnInvalidFunctionNameStaysOpenAndInvalid) {
  Harness h;
  h.send(down(kFnName));
  h.send(text("1"));  // 1f

  h.send(key(InputEvent::Key::Return));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_TRUE(h.tool.field()->invalid());
  EXPECT_EQ(h.fnName(), "f");
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(TextEditTool, APressOnACallsLabelOpensItsTarget) {
  Harness h;

  EXPECT_FALSE(h.send(down(kCallLabel)));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "g");
}

TEST(TextEditTool, APressOnACallsArgumentRowOpensItsName) {
  Harness h;

  EXPECT_FALSE(h.send(down(kCallArgRow1)));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "b");
}

TEST(TextEditTool, ReturnRenamesTheArgumentUndoably) {
  Harness h;
  h.send(down(kCallArgRow1));
  h.send(key(InputEvent::Key::End));
  h.send(text("c"));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Return)));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_EQ(h.argName(1), "bc");
  EXPECT_EQ(h.argName(0), "a");
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.argName(1), "b");
}

TEST(TextEditTool, AnInvalidArgumentNameStaysOpenAndInvalid) {
  Harness h;
  h.send(down(kCallArgRow1));
  h.send(text("-"));  // -b

  h.send(key(InputEvent::Key::Return));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_TRUE(h.tool.field()->invalid());
  EXPECT_EQ(h.argName(1), "b");
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(TextEditTool, RePressingTheOpenArgumentKeepsTheDraftAndAnotherArgumentOpensItself) {
  Harness h;
  h.send(down(kCallArgRow1));
  h.send(text("c"));

  h.send(down(kCallArgRow1 + Vec2{8, 0}));
  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "cb");
  EXPECT_EQ(h.tool.field()->caret(), 1u);

  h.send(down(kCallArgRow));
  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "a");
}

TEST(TextEditTool, APressOnAParameterRailOpensItsName) {
  Harness h;

  EXPECT_FALSE(h.send(down(kParamRail1)));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "y");
}

TEST(TextEditTool, ReturnRenamesTheParameterUndoably) {
  Harness h;
  h.send(down(kParamRail1));
  h.send(key(InputEvent::Key::End));
  h.send(text("z"));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Return)));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_EQ(h.paramName(1), "yz");
  EXPECT_EQ(h.paramName(0), "x");
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.paramName(1), "y");
}

TEST(TextEditTool, AnInvalidParameterNameStaysOpenAndInvalid) {
  Harness h;
  h.send(down(kParamRail1));
  h.send(key(InputEvent::Key::Home));
  h.send(text("1"));  // 1y

  h.send(key(InputEvent::Key::Return));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_TRUE(h.tool.field()->invalid());
  EXPECT_EQ(h.paramName(1), "y");
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(TextEditTool, APressOnAParameterTypeOpensNothing) {
  Harness h;

  h.send(down(kParamRail1Type));

  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, APressOnTheReturnRailOpensNothing) {
  Harness h;

  h.send(down(kReturnRail));

  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, TheParameterDraftClosesWhenItsFunctionVanishes) {
  Harness h;
  h.send(down(kParamRail1));
  ASSERT_TRUE(h.state.editor.apply(std::make_unique<DeleteTransaction>(kFn)));

  EXPECT_FALSE(h.send(text("7")));

  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, ReturnRetargetsTheCallUndoably) {
  Harness h;
  h.send(down(kCallLabel));
  h.send(key(InputEvent::Key::End));
  h.send(text("h"));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Return)));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_EQ(h.callTarget(), "gh");
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.callTarget(), "g");
}

TEST(TextEditTool, AnInvalidCallTargetStaysOpenAndInvalid) {
  Harness h;
  h.send(down(kCallLabel));
  h.send(text("-"));  // -g

  h.send(key(InputEvent::Key::Return));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_TRUE(h.tool.field()->invalid());
  EXPECT_EQ(h.callTarget(), "g");
  EXPECT_FALSE(h.state.editor.canUndo());
}

TEST(TextEditTool, TheNameDraftClosesWhenItsFunctionVanishes) {
  Harness h;
  h.send(down(kFnName));
  ASSERT_TRUE(h.state.editor.apply(std::make_unique<DeleteTransaction>(kFn)));

  EXPECT_FALSE(h.send(text("7")));

  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, NameDraftsAreDrawnOverTheirCommittedLabels) {
  struct Case {
    Vec2 at;
    Vec2 textAt;
    std::string_view committed;
    std::string_view draft;
    Rect cover;
  };
  for (const Case& c : {Case{kFnName, kFnName, "f", "xf", kHeaderRect},
                        Case{kCallLabel, kCallLabel, "g", "xg", kCallLabelRect},
                        Case{kParamRail1, kParamRail1Text, "y", "yx", kParamRail1Rect},
                        Case{kCallArgRow1, kCallArgRow1, "b", "xb", kCallArgRow1Rect}}) {
    Harness h;
    h.send(down(c.at));
    h.send(text("x"));

    const auto calls = h.draw();

    const long value = textIndex(calls, c.committed, c.textAt);
    const long draft = textIndex(calls, c.draft, c.textAt);
    ASSERT_GE(value, 0) << c.committed;
    ASSERT_GT(draft, value) << c.committed;
    const auto cover = std::find_if(calls.begin() + value + 1, calls.begin() + draft, [&](const testutil::DrawCall& d) {
      return d.op == testutil::DrawCall::Op::Fill && testutil::detail::rectNear(d.rect, c.cover, 1e-6);
    });
    EXPECT_NE(cover, calls.begin() + draft) << c.committed;
  }
}

TEST(TextEditTool, APressOnACommentOpensItsTextWithTheCaretAtThePress) {
  Harness h;

  EXPECT_FALSE(h.send(down(kCommentText)));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "hi there");
  EXPECT_EQ(h.tool.field()->caret(), 2u);
}

TEST(TextEditTool, WithoutTextLayoutACommentOpensWithTheCaretAtTheEnd) {
  Harness h;
  h.state.text = nullptr;

  h.send(down(kCommentText));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->caret(), 8u);
}

TEST(TextEditTool, ReturnEditsTheCommentUndoably) {
  Harness h;
  h.send(down(kCommentText));
  h.send(key(InputEvent::Key::End));
  h.send(text(", you! (ok?)"));

  EXPECT_TRUE(h.send(key(InputEvent::Key::Return)));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_EQ(h.commentText(kComment), "hi there, you! (ok?)");
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.commentText(kComment), "hi there");
}

TEST(TextEditTool, AnEmptyCommentCommits) {
  Harness h;
  h.send(down(kCommentText));
  h.send(key(InputEvent::Key::End));
  for (int i = 0; i < 8; ++i) {
    h.send(key(InputEvent::Key::Backspace));
  }

  h.send(key(InputEvent::Key::Return));

  EXPECT_EQ(h.tool.field(), nullptr);
  EXPECT_EQ(h.commentText(kComment), "");
}

TEST(TextEditTool, AnUnchangedCommentOrEscapeClosesWithoutAnEdit) {
  for (const InputEvent::Key k : {InputEvent::Key::Return, InputEvent::Key::Escape}) {
    Harness h;
    h.send(down(kCommentText));
    if (k == InputEvent::Key::Escape) {
      h.send(text("x"));
    }

    h.send(key(k));

    EXPECT_EQ(h.tool.field(), nullptr);
    EXPECT_EQ(h.commentText(kComment), "hi there");
    EXPECT_FALSE(h.state.editor.canUndo());
  }
}

TEST(TextEditTool, RePressingTheOpenCommentKeepsTheDraftAndReAimsTheCaret) {
  Harness h;
  h.send(down(kCommentText));
  h.send(text("x"));

  h.send(down(kCommentText + Vec2{16, 0}));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "hix there");
  EXPECT_EQ(h.tool.field()->caret(), 4u);
}

TEST(TextEditTool, TheCommentDraftClosesWhenTheCommentVanishes) {
  Harness h;
  h.send(down(kCommentText));
  ASSERT_TRUE(h.state.editor.apply(std::make_unique<DeleteTransaction>(kComment)));

  EXPECT_FALSE(h.send(text("7")));

  EXPECT_EQ(h.tool.field(), nullptr);
}

TEST(TextEditTool, AnInBodyCommentOpensAndCommits) {
  Harness h;
  h.send(down(kInnerCommentText));
  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "inner");

  h.send(key(InputEvent::Key::End));
  h.send(text(" text"));
  h.send(key(InputEvent::Key::Return));

  EXPECT_EQ(h.commentText(kInnerComment), "inner text");
  ASSERT_TRUE(h.state.editor.undo());
  EXPECT_EQ(h.commentText(kInnerComment), "inner");
}

TEST(TextEditTool, TheCommentDraftIsDrawnWrappedOverTheCommittedText) {
  Harness h;
  h.send(down(kCommentText));
  h.send(text("x"));

  const auto calls = h.draw();

  const auto wrappedIndex = [&](std::string_view s) {
    for (std::size_t i = 0; i < calls.size(); ++i) {
      if (calls[i].op == testutil::DrawCall::Op::TextWrapped && calls[i].text == s &&
          calls[i].rect == kCommentTextRect && calls[i].scale == 1.0) {
        return static_cast<long>(i);
      }
    }
    return -1L;
  };
  const long value = wrappedIndex("hi there");
  const long draft = wrappedIndex("hix there");
  ASSERT_GE(value, 0);
  ASSERT_GT(draft, value);
  const auto cover = std::find_if(calls.begin() + value + 1, calls.begin() + draft, [](const testutil::DrawCall& c) {
    return c.op == testutil::DrawCall::Op::Fill && c.rect == kCommentRect;
  });
  EXPECT_NE(cover, calls.begin() + draft);
  EXPECT_TRUE(testutil::hasFill(calls, h.recorder.wrappedCaretRect(kCommentTextRect, "hix there", 1.0, 3)));
}
