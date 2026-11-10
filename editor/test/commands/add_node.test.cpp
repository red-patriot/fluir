#include "editor/commands/add_node.hpp"

#include <optional>
#include <QGraphicsItem>
#include <stdexcept>
#include <type_traits>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/commands/place_node.hpp"
#include "editor/core/intelligence.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/ids.hpp"
#include "editor/diagram/node.hpp"
#include "interaction.hpp"

// Adding a completion puts its prototype in the scope at the given grid point, with the scope's next id, one above its
// container, at a size its kind allows. Undo takes it out; redo brings back the same item.

namespace {

  namespace diagram = fluir::editor::diagram;
  namespace pt = fluir::pt;
  using fluir::editor::AddNode;
  using fluir::editor::Vec2i;
  using testutil::Interaction;

  constexpr Vec2i kAt{3, 4};

  pt::Block& bodyOf(pt::ParseTree& tree) { return std::get<pt::FunctionDecl>(tree.declarations.at(1)).body; }

  // `prototype` with `placed`'s id and location.
  template <typename Variant>
  Variant placedLike(Variant prototype, const Variant& placed) {
    std::visit(
      [&placed](auto& proto) {
        const auto& with = std::get<std::decay_t<decltype(proto)>>(placed);
        proto.id = with.id;
        proto.location = with.location;
      },
      prototype);
    return prototype;
  }

  template <typename Variant>
  fluir::FlowGraphLocation locationOf(const Variant& item) {
    return std::visit([](const auto& n) { return n.location; }, item);
  }

  diagram::FluirNode& childNode(const QGraphicsItem& scope, fluir::ID id) {
    for (QGraphicsItem* child : scope.childItems()) {
      if (auto* node = dynamic_cast<diagram::FluirNode*>(child); node != nullptr && node->id() == id) {
        return *node;
      }
    }
    throw std::logic_error("no such node");
  }

  diagram::FluirNode& topLevelNode(fluir::editor::ModuleScene& scene, fluir::ID id) {
    for (QGraphicsItem* item : scene.items()) {
      if (auto* node = dynamic_cast<diagram::FluirNode*>(item);
          node != nullptr && node->parentItem() == nullptr && node->id() == id) {
        return *node;
      }
    }
    throw std::logic_error("no such node");
  }

  void expectSizeAllowed(const diagram::FluirNode& node) {
    const auto limits = node.sizeLimits();
    EXPECT_GE(node.location().width, limits.lower.x);
    EXPECT_GE(node.location().height, limits.lower.y);
    EXPECT_LE(node.location().width, limits.upper.x);
    EXPECT_LE(node.location().height, limits.upper.y);
  }

}  // namespace

TEST(AddNode, EachBodyCompletionAddsItsPrototypeAtThePoint) {
  Interaction h("interaction/fan_out.fl");
  auto& function = h.find<diagram::Function>(1);
  const pt::ParseTree before = h.tree();
  const auto completions = fluir::editor::Intelligence{}.completionsAt(before, {1});
  ASSERT_FALSE(completions.empty());

  for (const auto& completion : completions) {
    const fluir::ID id = diagram::nextId(*function.body());

    h.stack().push(new AddNode(*function.body(), completion.prototype, kAt));
    pt::ParseTree after = h.tree();
    ASSERT_TRUE(bodyOf(after).nodes.contains(id)) << completion.label;
    const pt::Node added = bodyOf(after).nodes.at(id);
    bodyOf(after).nodes.erase(id);

    EXPECT_EQ(after, before) << completion.label;
    EXPECT_EQ(added, placedLike(completion.prototype, added)) << completion.label;
    EXPECT_EQ(locationOf(added).x, kAt.x) << completion.label;
    EXPECT_EQ(locationOf(added).y, kAt.y) << completion.label;
    EXPECT_EQ(locationOf(added).z, function.location().z + 1) << completion.label;
    expectSizeAllowed(childNode(*function.body(), id));

    h.stack().undo();
    EXPECT_EQ(h.tree(), before) << completion.label;
  }
}

TEST(AddNode, EachTopLevelCompletionAddsItsPrototypeAtThePoint) {
  Interaction h("read/multiple_empty_functions.fl");
  const pt::ParseTree before = h.tree();
  const auto completions = fluir::editor::Intelligence{}.topLevelCompletions(before);
  ASSERT_FALSE(completions.empty());

  for (const auto& completion : completions) {
    const fluir::ID id = diagram::nextId(h.scene());

    h.stack().push(new AddNode(h.scene(), completion.prototype, kAt));
    pt::ParseTree after = h.tree();
    ASSERT_TRUE(after.declarations.contains(id)) << completion.label;
    const pt::Declaration added = after.declarations.at(id);
    after.declarations.erase(id);

    EXPECT_EQ(after, before) << completion.label;
    EXPECT_EQ(added, placedLike(completion.prototype, added)) << completion.label;
    EXPECT_EQ(locationOf(added).x, kAt.x) << completion.label;
    EXPECT_EQ(locationOf(added).y, kAt.y) << completion.label;
    EXPECT_EQ(locationOf(added).z, 0) << completion.label;
    expectSizeAllowed(topLevelNode(h.scene(), id));

    h.stack().undo();
    EXPECT_EQ(h.tree(), before) << completion.label;
  }
}

TEST(AddNode, InABranchItTakesTheBranchesNextId) {
  Interaction h("interaction/ported_conditional.fl");
  auto& conditional = h.find<diagram::Conditional>(2);
  diagram::Scope& branch = *conditional.thenBranch();
  const fluir::ID id = diagram::nextId(branch);
  const pt::Node comment = pt::Comment{.id = fluir::INVALID_ID, .location = {}, .text = ""};

  h.stack().push(new AddNode(branch, comment, kAt));

  pt::ParseTree after = h.tree();
  const auto& placed = std::get<pt::Conditional>(bodyOf(after).nodes.at(2));
  ASSERT_TRUE(placed.thenScope->nodes.contains(id));
  EXPECT_EQ(locationOf(placed.thenScope->nodes.at(id)).z, conditional.location().z + 1);
  EXPECT_FALSE(placed.elseScope->nodes.contains(id));
}

// Later commands name the added item, so redo must bring back the same one.
TEST(AddNode, RedoBringsBackTheSameItem) {
  Interaction h("interaction/fan_out.fl");
  auto& function = h.find<diagram::Function>(1);
  const fluir::ID id = diagram::nextId(*function.body());
  const pt::Node comment = pt::Comment{.id = fluir::INVALID_ID, .location = {}, .text = ""};
  h.stack().push(new AddNode(*function.body(), comment, kAt));
  auto& added = childNode(*function.body(), id);
  fluir::FlowGraphLocation moved = added.location();
  moved.x += 5;
  h.stack().push(new fluir::editor::PlaceNode(added, added.location(), moved));
  const pt::ParseTree after = h.tree();

  h.stack().undo();
  h.stack().undo();
  h.stack().redo();
  h.stack().redo();

  EXPECT_EQ(h.tree(), after);
  EXPECT_EQ(&childNode(*function.body(), id), &added);
}
