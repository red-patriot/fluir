#include "editor/tools/text_edit_tool.hpp"

#include <algorithm>
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
  const FullID kFn{1};
  const FullID kCall{1, 14};
  constexpr Vec2 kFnName{4, 4};
  constexpr Vec2 kCallLabel{154, 129};
  constexpr Vec2 kCallArgRow{154, 154};
  constexpr Vec2 kCallArgRow1{154, 179};
  constexpr Vec2 kParamRail1{4, 54};
  constexpr Vec2 kReturnRail{479, 29};
  const Rect kParamRail1Rect{0, 50, 75, 25};
  const Rect kCallArgRow1Rect{150, 175, 50, 25};
  const Rect kHeaderRect{0, 0, 500, 25};
  const Rect kCallLabelRect{150, 125, 50, 25};

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

TEST(TextEditTool, APressOnAFunctionHeaderOpensItsName) {
  Harness h;

  EXPECT_FALSE(h.send(down(kFnName)));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_EQ(h.tool.field()->text(), "f");
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
  h.send(text("1"));  // 1y

  h.send(key(InputEvent::Key::Return));

  ASSERT_NE(h.tool.field(), nullptr);
  EXPECT_TRUE(h.tool.field()->invalid());
  EXPECT_EQ(h.paramName(1), "y");
  EXPECT_FALSE(h.state.editor.canUndo());
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
    std::string_view committed;
    std::string_view draft;
    Rect cover;
  };
  for (const Case& c : {Case{kFnName, "f", "xf", kHeaderRect},
                        Case{kCallLabel, "g", "xg", kCallLabelRect},
                        Case{kParamRail1, "i32 y", "xy", kParamRail1Rect},
                        Case{kCallArgRow1, "b", "xb", kCallArgRow1Rect}}) {
    Harness h;
    h.send(down(c.at));
    h.send(text("x"));

    const auto calls = h.draw();

    const long value = textIndex(calls, c.committed, c.at);
    const long draft = textIndex(calls, c.draft, c.at);
    ASSERT_GE(value, 0) << c.committed;
    ASSERT_GT(draft, value) << c.committed;
    const auto cover = std::find_if(calls.begin() + value + 1, calls.begin() + draft, [&](const testutil::DrawCall& d) {
      return d.op == testutil::DrawCall::Op::Fill && d.rect == c.cover;
    });
    EXPECT_NE(cover, calls.begin() + draft) << c.committed;
  }
}
