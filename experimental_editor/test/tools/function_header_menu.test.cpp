#include "editor/tools/function_header_menu.hpp"

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/tools/context_menu_tool.hpp"
#include "editor/view/graph_layout.hpp"
#include "tool_harness.hpp"

// single_empty_function.fl: function 1 at world {50,50,500,500}, header band y 50..75.

namespace {

  using fluir::FullID;
  using fluir::editor::Box;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::functionAt;
  using fluir::editor::functionHeaderItems;
  using fluir::editor::hitAt;
  using fluir::editor::layoutGraph;
  using fluir::editor::MenuItem;
  using fluir::editor::Vec2;

  const EditorContext kCtx;
  constexpr Vec2 kHeader{60, 60};
  constexpr Vec2 kHeaderGrip{537.5, 62.5};
  constexpr Vec2 kBodyBelowHeader{60, 400};
  constexpr Vec2 kNodeBody{137, 97};  // simple_binary_expr.fl binary 1

  struct Fixture {
    EditorState state{kCtx};

    explicit Fixture(const std::string& program) { testutil::loadInto(state, program); }

    std::vector<MenuItem> itemsAt(Vec2 world) const {
      const std::vector<Box> boxes = layoutGraph(state.editor.tree(), kCtx.layout);
      const Box* hit = hitAt(boxes, world);
      return hit == nullptr ? std::vector<MenuItem>{} : functionHeaderItems(*hit, world, state);
    }
  };

  std::vector<std::string> labelsOf(const std::vector<MenuItem>& items) {
    std::vector<std::string> out;
    for (const MenuItem& item : items) {
      out.push_back(item.label);
    }
    return out;
  }

  const std::vector<std::string> kLabels{"Add parameter", "Add return"};

}  // namespace

TEST(FunctionHeaderMenu, AHeaderPressOffersAddParameterAndAddReturn) {
  Fixture f{"read/single_empty_function.fl"};

  for (const Vec2 at : {kHeader, kHeaderGrip}) {
    const std::vector<MenuItem> items = f.itemsAt(at);
    EXPECT_EQ(labelsOf(items), kLabels);
    for (const MenuItem& item : items) {
      EXPECT_TRUE(item.enabled) << item.label;
    }
  }
}

TEST(FunctionHeaderMenu, TheBodyNodesAndCommentsOfferNothing) {
  EXPECT_TRUE(Fixture{"read/single_empty_function.fl"}.itemsAt(kBodyBelowHeader).empty());
  EXPECT_TRUE(Fixture{"read/simple_binary_expr.fl"}.itemsAt(kNodeBody).empty());
  EXPECT_TRUE(Fixture{"read/top_level_comment_only.fl"}.itemsAt(kHeader).empty());
}

TEST(FunctionHeaderMenu, AddReturnIsDisabledWhenTheFunctionHasOne) {
  Fixture f{"read/function_with_output_only.fl"};

  const std::vector<MenuItem> items = f.itemsAt(kHeader);

  ASSERT_EQ(labelsOf(items), kLabels);
  EXPECT_TRUE(items[0].enabled);
  EXPECT_FALSE(items[1].enabled);
}

TEST(FunctionHeaderMenu, AddParameterAddsAnI32ParameterAsOneUndoableEdit) {
  Fixture f{"read/single_empty_function.fl"};
  const fluir::pt::ParseTree before = f.state.editor.tree();

  f.itemsAt(kHeader)[0].onClick(f.state);

  const auto* fn = functionAt(f.state.editor.tree(), FullID{1});
  ASSERT_TRUE(fn->input.has_value());
  ASSERT_EQ(fn->input->parameters.size(), 1u);
  EXPECT_EQ(fn->input->parameters[0].typeName, "I32");
  ASSERT_TRUE(f.state.editor.undo());
  EXPECT_EQ(f.state.editor.tree(), before);
}

TEST(FunctionHeaderMenu, AddReturnAddsAnI32ReturnAsOneUndoableEdit) {
  Fixture f{"read/single_empty_function.fl"};
  const fluir::pt::ParseTree before = f.state.editor.tree();

  f.itemsAt(kHeader)[1].onClick(f.state);

  const auto* fn = functionAt(f.state.editor.tree(), FullID{1});
  ASSERT_TRUE(fn->output.has_value() && fn->output->ret.has_value());
  EXPECT_EQ(fn->output->ret->typeName, "I32");
  ASSERT_TRUE(f.state.editor.undo());
  EXPECT_EQ(f.state.editor.tree(), before);
}
