#include "editor/tools/port_menu.hpp"

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
#include "editor/view/graph_layout.hpp"

namespace {

  using fluir::FullID;
  using fluir::editor::Box;
  using fluir::editor::EditorContext;
  using fluir::editor::EditorState;
  using fluir::editor::MenuItem;
  using fluir::editor::Part;
  using fluir::editor::PortRef;
  namespace et = fluir::editor::et;

  const EditorContext CTX;
  const FullID CONDITIONAL_ID{1, 2};
  constexpr PortRef CONDITION{.output = false, .index = 0};
  constexpr PortRef INPUT{.output = false, .index = 1};
  constexpr PortRef OUTPUT{.output = true, .index = 0};

  struct Fixture {
    EditorState state{CTX};

    Fixture() {
      et::Conditional conditional{.id = 2,
                                  .location = {.x = 2, .y = 2, .z = 0, .width = 20, .height = 20},
                                  .condition = {.innerId = 1, .y = 2},
                                  .inputs = {{.innerId = 3, .y = 6}},
                                  .outputs = {{.innerId = 4, .y = 6}},
                                  .thenScope = xyz::indirect<et::Block>{},
                                  .elseScope = xyz::indirect<et::Block>{}};
      et::FunctionDecl fn;
      fn.id = 1;
      fn.location = {.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
      fn.name = "f";
      fn.body.nodes.emplace(2, std::move(conditional));
      et::ParseTree tree;
      tree.declarations.emplace(1, et::Declaration{std::move(fn)});
      state.editor.load(std::nullopt, std::move(tree));
    }

    std::optional<Box> boxOf(const FullID& path, Part part, std::optional<PortRef> port = std::nullopt) const {
      for (const Box& box : layoutGraph(state.editor.tree(), CTX.layout)) {
        if (box.path == path && box.part == part && box.port == port) {
          return box;
        }
      }
      return std::nullopt;
    }

    std::vector<MenuItem> itemsOn(const FullID& path, Part part, std::optional<PortRef> port = std::nullopt) const {
      const std::optional<Box> box = boxOf(path, part, port);
      EXPECT_TRUE(box.has_value());
      return box ? fluir::editor::portItems(*box, state) : std::vector<MenuItem>{};
    }

    const et::Conditional& conditional() const {
      return std::get<et::Conditional>(*fluir::editor::nodeAt(state.editor.tree(), CONDITIONAL_ID));
    }
  };

}  // namespace

TEST(PortMenu, InputAndOutputPortsOfferDeletePort) {
  const Fixture f;

  for (const PortRef ref : {INPUT, OUTPUT}) {
    const std::vector<MenuItem> items = f.itemsOn(CONDITIONAL_ID, Part::Port, ref);
    ASSERT_EQ(items.size(), 1u) << ref.output;
    EXPECT_EQ(items[0].label, "Delete port");
    EXPECT_TRUE(items[0].enabled);
  }
}

TEST(PortMenu, TheConditionPortHeaderAndBodyOfferNothing) {
  const Fixture f;

  EXPECT_TRUE(f.itemsOn(CONDITIONAL_ID, Part::Port, CONDITION).empty()) << "condition";
  EXPECT_TRUE(f.itemsOn(CONDITIONAL_ID, Part::Header).empty()) << "header";
  EXPECT_TRUE(f.itemsOn(CONDITIONAL_ID, Part::Body).empty()) << "body";
}

TEST(PortMenu, DeletePortRemovesThatPortAsOneUndoableEdit) {
  for (const PortRef ref : {INPUT, OUTPUT}) {
    Fixture f;
    const et::ParseTree before = f.state.editor.tree();

    f.itemsOn(CONDITIONAL_ID, Part::Port, ref)[0].onClick(f.state);

    EXPECT_EQ(f.conditional().inputs.size(), ref.output ? 1u : 0u) << ref.output;
    EXPECT_EQ(f.conditional().outputs.size(), ref.output ? 0u : 1u) << ref.output;
    ASSERT_TRUE(f.state.editor.undo());
    EXPECT_EQ(f.state.editor.tree(), before);
  }
}
