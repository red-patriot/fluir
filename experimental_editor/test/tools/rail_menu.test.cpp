#include "editor/tools/rail_menu.hpp"

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

// Rail points are taken from the laid-out Rail boxes, so they track layout changes.

namespace {

  using fluir::FullID;
  using fluir::editor::Box;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::functionAt;
  using fluir::editor::hitAt;
  using fluir::editor::layoutGraph;
  using fluir::editor::MenuItem;
  using fluir::editor::Part;
  using fluir::editor::railItems;
  using fluir::editor::Vec2;

  const EditorContext kCtx;
  constexpr Vec2 kHeader{60, 60};
  constexpr Vec2 kBodyBelowHeader{300, 400};
  constexpr Vec2 kNodeBody{137, 97};  // simple_binary_expr.fl binary 1

  struct Fixture {
    EditorState state{kCtx};

    explicit Fixture(const std::string& program) { testutil::loadInto(state, program); }

    std::vector<MenuItem> itemsAt(Vec2 world) const {
      const std::vector<Box> boxes = layoutGraph(state.editor.tree(), kCtx.layout);
      const Box* hit = hitAt(boxes, world);
      return hit == nullptr ? std::vector<MenuItem>{} : railItems(*hit, state);
    }

    // The center of the rail box laid out for `path`.
    Vec2 railCenter(const FullID& path) const {
      for (const Box& box : layoutGraph(state.editor.tree(), kCtx.layout)) {
        if (box.part == Part::Rail && box.path == path) {
          return box.world.center();
        }
      }
      ADD_FAILURE() << "no rail laid out";
      return {};
    }
  };

  std::vector<std::string> labelsOf(const std::vector<MenuItem>& items) {
    std::vector<std::string> out;
    for (const MenuItem& item : items) {
      out.push_back(item.label);
    }
    return out;
  }

  const std::vector<std::string> kDelete{"Delete"};

}  // namespace

TEST(RailMenu, AParameterOrReturnRailOffersDelete) {
  Fixture params{"read/function_with_input_only.fl"};
  Fixture ret{"read/function_with_output_only.fl"};

  EXPECT_EQ(labelsOf(params.itemsAt(params.railCenter(FullID{1, 2}))), kDelete);
  EXPECT_EQ(labelsOf(ret.itemsAt(ret.railCenter(FullID{1, 4}))), kDelete);
}

TEST(RailMenu, TheHeaderBodyNodesAndCommentsOfferNothing) {
  Fixture f{"read/function_with_input_only.fl"};

  EXPECT_TRUE(f.itemsAt(kHeader).empty());
  EXPECT_TRUE(f.itemsAt(kBodyBelowHeader).empty());
  EXPECT_TRUE(Fixture{"read/simple_binary_expr.fl"}.itemsAt(kNodeBody).empty());
  EXPECT_TRUE(Fixture{"read/top_level_comment_only.fl"}.itemsAt(kHeader).empty());
}

TEST(RailMenu, DeleteRemovesTheParameterAsOneUndoableEdit) {
  Fixture f{"read/function_with_input_only.fl"};
  const fluir::pt::ParseTree before = f.state.editor.tree();

  f.itemsAt(f.railCenter(FullID{1, 2})).at(0).onClick(f.state);

  const auto& params = functionAt(f.state.editor.tree(), FullID{1})->input->parameters;
  ASSERT_EQ(params.size(), 1u);
  EXPECT_EQ(params[0].id, 3);
  ASSERT_TRUE(f.state.editor.undo());
  EXPECT_EQ(f.state.editor.tree(), before);
}

TEST(RailMenu, DeleteRemovesTheReturnAsOneUndoableEdit) {
  Fixture f{"read/function_with_output_only.fl"};
  const fluir::pt::ParseTree before = f.state.editor.tree();

  f.itemsAt(f.railCenter(FullID{1, 4})).at(0).onClick(f.state);

  EXPECT_FALSE(functionAt(f.state.editor.tree(), FullID{1})->output->ret.has_value());
  ASSERT_TRUE(f.state.editor.undo());
  EXPECT_EQ(f.state.editor.tree(), before);
}
