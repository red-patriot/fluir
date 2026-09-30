#include "editor/tools/conditional_header_menu.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/tools/context_menu_tool.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/graph_layout.hpp"

namespace {

  using fluir::FullID;
  using fluir::editor::Box;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::ELSE_BRANCH_ID;
  using fluir::editor::MenuItem;
  using fluir::editor::Part;
  using fluir::editor::THEN_BRANCH_ID;
  namespace et = fluir::editor::et;

  const EditorContext kCtx;
  const FullID kConditional{1, 2};

  struct Fixture {
    EditorState state{kCtx};

    Fixture() {
      et::Block then;
      then.nodes.emplace(30, et::Comment{.id = 30, .location = {.x = 1, .y = 4, .z = 0, .width = 5, .height = 5}});
      et::Block otherwise;
      otherwise.nodes.emplace(40, et::Comment{.id = 40, .location = {.x = 1, .y = 4, .z = 0, .width = 5, .height = 5}});
      et::Conditional conditional{.id = 2,
                                  .location = {.x = 2, .y = 2, .z = 0, .width = 20, .height = 20},
                                  .condition = {.innerId = 1, .y = 2},
                                  .inputs = {},
                                  .outputs = {},
                                  .thenScope = xyz::indirect{std::move(then)},
                                  .elseScope = xyz::indirect{std::move(otherwise)}};
      et::FunctionDecl fn;
      fn.id = 1;
      fn.location = {.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
      fn.name = "f";
      fn.body.nodes.emplace(2, std::move(conditional));
      et::ParseTree tree;
      tree.declarations.emplace(1, et::Declaration{std::move(fn)});
      state.editor.load(std::nullopt, std::move(tree));
    }

    std::optional<Box> boxOf(const FullID& path, Part part) const {
      for (const Box& box : layoutGraph(state.editor.tree(), kCtx.layout)) {
        if (box.path == path && box.part == part) {
          return box;
        }
      }
      return std::nullopt;
    }

    std::vector<MenuItem> itemsOn(const FullID& path, Part part) const {
      const std::optional<Box> box = boxOf(path, part);
      return box ? fluir::editor::conditionalHeaderItems(*box, state) : std::vector<MenuItem>{};
    }

    const et::Conditional& conditional() const {
      return std::get<et::Conditional>(*fluir::editor::nodeAt(state.editor.tree(), kConditional));
    }
  };

  std::vector<std::string> labelsOf(const std::vector<MenuItem>& items) {
    std::vector<std::string> out;
    for (const MenuItem& item : items) {
      out.push_back(item.label);
    }
    return out;
  }

  const std::vector<std::string> kLabels{"Add input", "Add output"};

}  // namespace

TEST(ConditionalHeaderMenu, AHeaderPressOffersAddInputAndAddOutput) {
  const Fixture f;

  const std::vector<MenuItem> items = f.itemsOn(kConditional, Part::Header);

  EXPECT_EQ(labelsOf(items), kLabels);
  for (const MenuItem& item : items) {
    EXPECT_TRUE(item.enabled) << item.label;
  }
}

TEST(ConditionalHeaderMenu, FunctionHeadersBodiesAndNodesOfferNothing) {
  const Fixture f;

  ASSERT_TRUE(f.boxOf(FullID{1}, Part::Header).has_value());
  EXPECT_TRUE(f.itemsOn(FullID{1}, Part::Header).empty()) << "function header";
  ASSERT_TRUE(f.boxOf(kConditional, Part::Body).has_value());
  EXPECT_TRUE(f.itemsOn(kConditional, Part::Body).empty()) << "conditional body";
  ASSERT_TRUE(f.boxOf(FullID{1, 2, THEN_BRANCH_ID, 30}, Part::Body).has_value());
  EXPECT_TRUE(f.itemsOn(FullID{1, 2, THEN_BRANCH_ID, 30}, Part::Body).empty()) << "node in a branch";
}

TEST(ConditionalHeaderMenu, EachItemAddsOneFreeMidWallPortAsOneUndoableEdit) {
  for (const bool output : {false, true}) {
    Fixture f;
    const et::ParseTree before = f.state.editor.tree();

    f.itemsOn(kConditional, Part::Header)[output ? 1 : 0].onClick(f.state);

    const et::Conditional& conditional = f.conditional();
    const std::vector<et::BlockPort>& added = output ? conditional.outputs : conditional.inputs;
    const std::vector<et::BlockPort>& other = output ? conditional.inputs : conditional.outputs;
    ASSERT_EQ(added.size(), 1u) << output;
    EXPECT_TRUE(other.empty()) << output;
    const auto limits = fluir::editor::draw::portYLimits(conditional, kCtx.layout);
    EXPECT_GE(added[0].y, limits.lower) << output;
    EXPECT_LE(added[0].y, limits.upper) << output;
    for (const fluir::ID branch : {THEN_BRANCH_ID, ELSE_BRANCH_ID}) {
      const et::Block* block = fluir::editor::branchBlock(conditional, branch);
      EXPECT_FALSE(block->nodes.contains(added[0].innerId)) << output << " branch " << branch;
      EXPECT_FALSE(block->conduits.contains(added[0].innerId)) << output << " branch " << branch;
    }
    EXPECT_NE(added[0].innerId, conditional.condition.innerId) << output;

    ASSERT_TRUE(f.state.editor.undo());
    EXPECT_EQ(f.state.editor.tree(), before);
  }
}
