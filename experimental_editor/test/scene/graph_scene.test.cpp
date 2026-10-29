#include "editor/scene/graph_scene.hpp"

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/field.hpp"
#include "editor/core/fields.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/scene/components/frame.hpp"
#include "editor/scene/elements/binary_element.hpp"
#include "editor/scene/elements/call_element.hpp"
#include "editor/scene/elements/comment_element.hpp"
#include "editor/scene/elements/conditional_element.hpp"
#include "editor/scene/elements/constant_element.hpp"
#include "editor/scene/elements/function_element.hpp"
#include "editor/scene/elements/unary_element.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/transaction/move.hpp"
#include "editor/transaction/move_port.hpp"
#include "editor/transaction/rename.hpp"
#include "editor/transaction/resize.hpp"
#include "editor/transaction/set_constant_value.hpp"
#include "editor/transaction/transaction.hpp"
#include "editor/view/graph_layout.hpp"
#include "file_utility.hpp"
#include "fixture_loader.hpp"
#include "scene_probe.hpp"

// The scene is a keyed projection of the tree: one item per path, laid out by the pure kind geometry, and the same
// after any edit as a scene built fresh from the edited tree.

namespace {

  namespace fs = std::filesystem;

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  using fluir::ID;
  using fluir::editor::EditorContext;
  using fluir::editor::Element;
  using fluir::editor::GraphScene;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;
  namespace et = fluir::editor::et;
  using testutil::componentsOf;
  using testutil::itemsOf;
  using testutil::toRect;
  using testutil::toVec;

  const EditorContext kCtx;

  et::ParseTree loadProgram(const fs::path& program) {
    testutil::Loaded loaded = testutil::loadFixture(program.string());
    EXPECT_TRUE(loaded.result.tree.has_value()) << program;
    return loaded.result.tree.value_or(et::ParseTree{});
  }

  et::ParseTree loadTree(const std::string& relPath) { return loadProgram(fs::path(TEST_FOLDER) / relPath); }

  et::Constant makeConstant(ID id, FlowGraphLocation loc) {
    return et::Constant{.id = id, .location = loc, .value = fluir::literals_types::I32{0}};
  }

  // A 100x100-unit function at the origin.
  et::ParseTree functionWith(std::vector<et::Node> nodes) {
    et::FunctionDecl fn;
    fn.id = 1;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    for (et::Node& node : nodes) {
      const ID id = std::visit([](const auto& n) { return n.id; }, node);
      fn.body.nodes.emplace(id, std::move(node));
    }
    et::ParseTree tree;
    tree.declarations.emplace(fn.id, et::Declaration{std::move(fn)});
    return tree;
  }

  void apply(et::ParseTree& tree, std::unique_ptr<fluir::editor::Transaction> edit) {
    ASSERT_TRUE(edit->execute(tree));
  }

  void showBranch(et::ParseTree& tree, const FullID& conditional, ID branch) {
    std::get<et::Conditional>(*fluir::editor::nodeAt(tree, conditional)).annotation.shownBranch = branch;
  }

  // The topmost item at `world` (scene px), or null.
  const QGraphicsItem* topAt(const GraphScene& scene, Vec2 world) {
    const QList<QGraphicsItem*> hits = scene.items(QPointF(world.x, world.y));
    return hits.isEmpty() ? nullptr : hits.front();
  }

  // The block `item` draws for: itself, or the block owning it as a component.
  const Element* blockFor(const QGraphicsItem* item) {
    if (const auto* block = dynamic_cast<const Element*>(item)) {
      return block;
    }
    return item == nullptr ? nullptr : testutil::ownerOf(item);
  }

  bool inside(const Rect& inner, const Rect& outer, double tol = 1e-6) {
    return inner.x >= outer.x - tol && inner.y >= outer.y - tol && inner.x + inner.w <= outer.x + outer.w + tol &&
           inner.y + inner.h <= outer.y + outer.h + tol;
  }

  bool near(Vec2 a, Vec2 b, double tol = 1e-6) { return std::abs(a.x - b.x) <= tol && std::abs(a.y - b.y) <= tol; }

  const fluir::editor::Terminal* terminal(const GraphScene& scene, const FullID& path, bool output, int index) {
    for (const auto* t : itemsOf<fluir::editor::Terminal>(scene)) {
      if (t->modelPath() == path && t->output() == output && t->index() == static_cast<std::size_t>(index)) {
        return t;
      }
    }
    return nullptr;
  }

  // ---- Invariants every read fixture keeps ----

  // read/ fixtures that fail to load on purpose (see loader.test.cpp).
  std::vector<fs::path> loadableFixtures() {
    std::vector<fs::path> out = fluir::test::getTestPrograms("read");
    std::erase_if(out, [](const fs::path& p) {
      return p.stem() == "branching_conduits" || p.stem() == "function_with_input_and_output";
    });
    return out;
  }

  class EveryFixture : public ::testing::TestWithParam<fs::path> {
   protected:
    void SetUp() override {
      tree = loadProgram(GetParam());
      scene.reconcile(tree);
    }

    et::ParseTree tree;
    GraphScene scene{kCtx};
  };

  TEST_P(EveryFixture, EveryPathHasAnItemAndNoItemOutlivesItsPath) {
    const testutil::TreePaths paths = testutil::treePaths(tree);

    for (const FullID& path : paths.blocks) {
      const Element* item = scene.itemAt(path);
      ASSERT_NE(item, nullptr) << testutil::str(path);
      EXPECT_EQ(item->modelPath(), path);
    }
    for (const FullID& path : paths.conduits) {
      ASSERT_NE(scene.conduitAt(path), nullptr) << testutil::str(path);
    }
    EXPECT_EQ(itemsOf<Element>(scene).size(), paths.blocks.size());
    EXPECT_EQ(itemsOf<fluir::editor::ConduitElement>(scene).size(), paths.conduits.size());
  }

  TEST_P(EveryFixture, EachItemIsItsNodesKind) {
    for (const FullID& path : testutil::treePaths(tree).blocks) {
      const Element* item = scene.itemAt(path);
      ASSERT_NE(item, nullptr) << testutil::str(path);
      if (path.size() == 1) {
        const et::Declaration& decl = tree.declarations.at(path[0]);
        EXPECT_EQ(dynamic_cast<const fluir::editor::FunctionElement*>(item) != nullptr,
                  std::holds_alternative<et::FunctionDecl>(decl));
        EXPECT_EQ(dynamic_cast<const fluir::editor::CommentElement*>(item) != nullptr,
                  std::holds_alternative<et::Comment>(decl));
      } else if (fluir::editor::isBranchPath(path)) {
        EXPECT_NE(dynamic_cast<const fluir::editor::BranchElement*>(item), nullptr);
      } else if (const et::Node* node = fluir::editor::nodeAt(tree, path)) {
        const bool matches = std::visit(
          [item]<typename N>(const N&) {
            if constexpr (std::is_same_v<N, et::Constant>)
              return dynamic_cast<const fluir::editor::ConstantElement*>(item) != nullptr;
            if constexpr (std::is_same_v<N, et::Binary>)
              return dynamic_cast<const fluir::editor::BinaryElement*>(item) != nullptr;
            if constexpr (std::is_same_v<N, et::Unary>)
              return dynamic_cast<const fluir::editor::UnaryElement*>(item) != nullptr;
            if constexpr (std::is_same_v<N, et::Call>)
              return dynamic_cast<const fluir::editor::CallElement*>(item) != nullptr;
            if constexpr (std::is_same_v<N, et::Comment>)
              return dynamic_cast<const fluir::editor::CommentElement*>(item) != nullptr;
            if constexpr (std::is_same_v<N, et::Conditional>) {
              return dynamic_cast<const fluir::editor::ConditionalElement*>(item) != nullptr;
            }
          },
          *node);
        EXPECT_TRUE(matches) << testutil::str(path);
      } else {
        EXPECT_NE(dynamic_cast<const fluir::editor::RailElement*>(item), nullptr) << testutil::str(path);
      }
    }
  }

  // Temporary parity with the SDL layout: retire with layoutGraph in the cleanup step.
  TEST_P(EveryFixture, MatchesTheSdlLayoutPartForPart) {
    std::vector<std::string> want;
    for (const fluir::editor::Box& box : fluir::editor::layoutGraph(tree, kCtx.layout)) {
      if (std::string key = testutil::layoutKey(box); !key.empty()) want.push_back(std::move(key));
    }
    std::vector<std::string> got;
    for (const QGraphicsItem* item : scene.items()) {
      if (std::string key = testutil::layoutKey(item); !key.empty()) got.push_back(std::move(key));
    }
    std::ranges::sort(want);
    std::ranges::sort(got);
    EXPECT_EQ(got, want);
  }

  TEST_P(EveryFixture, ComponentsLieInsideTheirElement) {
    for (const Element* block : itemsOf<Element>(scene)) {
      const Rect bounds = toRect(block->sceneBoundingRect());
      for (const QGraphicsItem* component : componentsOf<QGraphicsItem>(block)) {
        // Terminals and ports straddle walls; conduits belong to their scope, not to a body.
        if (dynamic_cast<const fluir::editor::Terminal*>(component) ||
            dynamic_cast<const fluir::editor::Port*>(component) ||
            dynamic_cast<const fluir::editor::ConduitElement*>(component)) {
          continue;
        }
        EXPECT_TRUE(inside(toRect(component->sceneBoundingRect()), bounds))
          << testutil::str(block->modelPath()) << " " << typeid(*component).name();
      }
    }
  }

  TEST_P(EveryFixture, NodeAndRailTerminalsSitOnTheirWalls) {
    for (const Element* block : itemsOf<Element>(scene)) {
      if (dynamic_cast<const fluir::editor::ConditionalElement*>(block) ||
          dynamic_cast<const fluir::editor::BranchElement*>(block)) {
        continue;  // port terminals sit off the wall, by design
      }
      const Rect r = toRect(block->sceneBoundingRect());
      for (const auto* t : componentsOf<fluir::editor::Terminal>(block)) {
        const Vec2 c = toRect(t->sceneBoundingRect()).center();
        EXPECT_NEAR(c.x, t->output() ? r.x + r.w : r.x, 1e-6) << testutil::str(block->modelPath());
        EXPECT_GE(c.y, r.y);
        EXPECT_LE(c.y, r.y + r.h);
      }
    }
  }

  TEST_P(EveryFixture, ConduitsRunFromTheirSourceTerminalToEachSink) {
    for (const FullID& path : testutil::treePaths(tree).conduits) {
      const FullID scope = fluir::editor::parentOf(path);
      const et::Block* block = fluir::editor::blockOf(tree, scope);
      ASSERT_NE(block, nullptr);
      const et::Conduit& conduit = block->conduits.at(path.back());
      const auto endpoint = [&scope](ID id) {
        FullID out = scope;
        out.push_back(id);
        return out;
      };
      const auto* source = terminal(scene, endpoint(conduit.input), true, conduit.index);
      std::vector<std::pair<Vec2, Vec2>> want;
      for (const et::Conduit::Output& target : conduit.children) {
        const auto* sink = terminal(scene, endpoint(target.target), false, target.index);
        if (source != nullptr && sink != nullptr) {
          want.emplace_back(toRect(source->sceneBoundingRect()).center(), toRect(sink->sceneBoundingRect()).center());
        }
      }
      const auto* item = scene.conduitAt(path);
      ASSERT_NE(item, nullptr) << testutil::str(path);
      const auto got = testutil::wiresOf(*item);
      ASSERT_EQ(got.size(), want.size()) << testutil::str(path);
      for (std::size_t i = 0; i < got.size(); ++i) {
        EXPECT_TRUE(near(got[i].first, want[i].first) && near(got[i].second, want[i].second)) << testutil::str(path);
      }
    }
  }

  TEST_P(EveryFixture, LabelsShowTheirFieldsValues) {
    for (const auto* label : itemsOf<fluir::editor::TextEdit>(scene)) {
      EXPECT_FALSE(fluir::editor::isChoice(label->field().kind));
      EXPECT_EQ(label->text().toStdString(), fluir::editor::fields::read(tree, label->modelPath(), label->field()));
    }
    for (const auto* label : itemsOf<fluir::editor::Choice>(scene)) {
      EXPECT_TRUE(fluir::editor::isChoice(label->field().kind));
      EXPECT_EQ(label->text().toStdString(), fluir::editor::fields::read(tree, label->modelPath(), label->field()));
    }
    for (const auto* toggle : itemsOf<fluir::editor::Toggle>(scene)) {
      const auto& constant = std::get<et::Constant>(*fluir::editor::nodeAt(tree, toggle->modelPath()));
      EXPECT_EQ(toggle->value(), std::get<fluir::literals_types::BOOL>(constant.value));
    }
  }

  TEST_P(EveryFixture, ReconcilingTheSameTreeAgainChangesNothing) {
    const std::vector<std::string> before = testutil::snapshot(scene);
    std::vector<const Element*> items;
    for (const FullID& path : testutil::treePaths(tree).blocks)
      items.push_back(scene.itemAt(path));

    scene.reconcile(tree);

    EXPECT_EQ(testutil::snapshot(scene), before);
    std::size_t i = 0;
    for (const FullID& path : testutil::treePaths(tree).blocks)
      EXPECT_EQ(scene.itemAt(path), items[i++]);
  }

  INSTANTIATE_TEST_SUITE_P(GraphScene,
                           EveryFixture,
                           ::testing::ValuesIn(loadableFixtures()),
                           fluir::test::filePathName);

  // ---- Reconciling after an edit ----

  struct Edit {
    std::string name;
    std::string fixture;
    std::function<void(et::ParseTree&)> change;
  };

  class AfterAnEdit : public ::testing::TestWithParam<Edit> { };

  TEST_P(AfterAnEdit, TheSceneMatchesOneBuiltFresh) {
    et::ParseTree tree = loadTree(GetParam().fixture);
    GraphScene reconciled{kCtx};
    reconciled.reconcile(tree);

    GetParam().change(tree);
    reconciled.reconcile(tree);
    GraphScene fresh{kCtx};
    fresh.reconcile(tree);

    EXPECT_EQ(testutil::snapshot(reconciled), testutil::snapshot(fresh));
  }

  INSTANTIATE_TEST_SUITE_P(
    GraphScene,
    AfterAnEdit,
    ::testing::Values(
      Edit{"MoveANode",
           "read/simple_binary_expr.fl",
           [](et::ParseTree& t) { apply(t, fluir::editor::moveTo({1, 2}, 7, 9)); }},
      Edit{"MoveAFunction",
           "read/conditional_with_body.fl",
           [](et::ParseTree& t) { apply(t, fluir::editor::moveTo({1}, 4, 4)); }},
      Edit{"ResizeAConditional",
           "read/conditional_with_body.fl",
           [](et::ParseTree& t) { apply(t, fluir::editor::resizeTo({1, 2}, 300, 200)); }},
      Edit{"DeleteANodeAndItsConduits",
           "read/simple_binary_expr.fl",
           [](et::ParseTree& t) { apply(t, fluir::editor::deleteAt({1, 2})); }},
      Edit{"DeleteAFunction",
           "read/multiple_empty_functions.fl",
           [](et::ParseTree& t) { apply(t, fluir::editor::deleteAt({1})); }},
      Edit{"SetAConstant",
           "read/int_constants.fl",
           [](et::ParseTree& t) { apply(t, fluir::editor::setConstantValue({1, 1}, fluir::literals_types::I8{42})); }},
      Edit{"RenameAFunction",
           "read/function_with_input_only.fl",
           [](et::ParseTree& t) { apply(t, fluir::editor::renameFunction({1}, "renamed")); }},
      Edit{"ShowTheElseBranch",
           "read/conditional_with_body.fl",
           [](et::ParseTree& t) { showBranch(t, {1, 2}, fluir::editor::ELSE_BRANCH_ID); }},
      Edit{"SlideTheConditionPort",
           "read/conditional_with_body.fl",
           [](et::ParseTree& t) { apply(t, fluir::editor::movePortTo({1, 2}, fluir::editor::PortRef{}, 30)); }},
      Edit{"ChangeANodesKind",
           "read/simple_binary_expr.fl",
           [](et::ParseTree& t) {
             auto& body = std::get<et::FunctionDecl>(t.declarations.at(1)).body;
             const FlowGraphLocation loc = std::get<et::Constant>(body.nodes.at(2)).location;
             body.nodes.at(2) = et::Unary{.id = 2, .location = loc, .op = fluir::Operator::MINUS};
           }}),
    [](const ::testing::TestParamInfo<Edit>& info) { return info.param.name; });

  // ---- Targeted contracts ----

  TEST(GraphScene, AnEmptyTreeHasNoItems) {
    GraphScene scene{kCtx};

    scene.reconcile(et::ParseTree{});

    EXPECT_TRUE(scene.items().isEmpty());
  }

  TEST(GraphScene, AnItemWhosePathSurvivesIsKeptAndMoved) {
    et::ParseTree tree = loadTree("read/simple_binary_expr.fl");
    GraphScene scene{kCtx};
    scene.reconcile(tree);
    const Element* before = scene.itemAt({1, 2});
    ASSERT_NE(before, nullptr);
    const QRectF rect = before->sceneBoundingRect();

    apply(tree, fluir::editor::moveTo({1, 2}, 7, 9));
    scene.reconcile(tree);

    ASSERT_EQ(scene.itemAt({1, 2}), before);
    EXPECT_NE(before->sceneBoundingRect(), rect);
  }

  TEST(GraphScene, ADeletedNodeTakesItsItemAndItsConduitsWithIt) {
    et::ParseTree tree = loadTree("read/simple_binary_expr.fl");
    GraphScene scene{kCtx};
    scene.reconcile(tree);
    ASSERT_NE(scene.conduitAt({1, 4}), nullptr);

    apply(tree, fluir::editor::deleteAt({1, 2}));
    scene.reconcile(tree);

    EXPECT_EQ(scene.itemAt({1, 2}), nullptr);
    EXPECT_EQ(scene.conduitAt({1, 4}), nullptr);
    EXPECT_NE(scene.conduitAt({1, 5}), nullptr);
  }

  TEST(GraphScene, AnItemWhoseKindChangedIsRecreatedAsTheNewKind) {
    et::ParseTree tree = loadTree("read/simple_binary_expr.fl");
    GraphScene scene{kCtx};
    scene.reconcile(tree);
    auto& body = std::get<et::FunctionDecl>(tree.declarations.at(1)).body;
    const FlowGraphLocation loc = std::get<et::Constant>(body.nodes.at(2)).location;

    body.nodes.at(2) = et::Unary{.id = 2, .location = loc, .op = fluir::Operator::MINUS};
    scene.reconcile(tree);

    EXPECT_NE(dynamic_cast<const fluir::editor::UnaryElement*>(scene.itemAt({1, 2})), nullptr);
  }

  TEST(GraphScene, OnlyTheShownBranchIsBuilt) {
    et::ParseTree tree = loadTree("read/conditional_with_body.fl");
    GraphScene scene{kCtx};
    scene.reconcile(tree);
    const FullID thenBranch{1, 2, fluir::editor::THEN_BRANCH_ID};
    const FullID elseBranch{1, 2, fluir::editor::ELSE_BRANCH_ID};
    ASSERT_NE(scene.itemAt(thenBranch), nullptr);
    EXPECT_EQ(scene.itemAt(elseBranch), nullptr);

    showBranch(tree, {1, 2}, fluir::editor::ELSE_BRANCH_ID);
    scene.reconcile(tree);

    EXPECT_EQ(scene.itemAt(thenBranch), nullptr);
    EXPECT_EQ(scene.itemAt({1, 2, fluir::editor::THEN_BRANCH_ID, 1}), nullptr);
    ASSERT_NE(scene.itemAt(elseBranch), nullptr);
    EXPECT_NE(dynamic_cast<const fluir::editor::BinaryElement*>(scene.itemAt({1, 2, fluir::editor::ELSE_BRANCH_ID, 3})),
              nullptr);
  }

  TEST(GraphScene, NodesNestInAClippingScopeOfTheirFunction) {
    GraphScene scene{kCtx};
    scene.reconcile(loadTree("read/simple_binary_expr.fl"));

    ASSERT_NE(scene.itemAt({1, 2}), nullptr);
    ASSERT_NE(scene.conduitAt({1, 4}), nullptr);
    const QGraphicsItem* scope = scene.itemAt({1, 2})->parentItem();

    ASSERT_NE(scope, nullptr);
    EXPECT_TRUE(scope->flags() & QGraphicsItem::ItemClipsChildrenToShape);
    EXPECT_EQ(scope->parentItem(), scene.itemAt({1}));
    EXPECT_EQ(scene.conduitAt({1, 4})->parentItem(), scope);
  }

  TEST(GraphScene, NestedNodesNestInTheirClippingBranch) {
    GraphScene scene{kCtx};
    scene.reconcile(loadTree("read/conditional_with_body.fl"));
    const Element* branch = scene.itemAt({1, 2, fluir::editor::THEN_BRANCH_ID});
    ASSERT_NE(branch, nullptr);
    const Element* nested = scene.itemAt({1, 2, fluir::editor::THEN_BRANCH_ID, 1});
    ASSERT_NE(nested, nullptr);

    EXPECT_EQ(nested->parentItem(), branch);
    EXPECT_TRUE(branch->flags() & QGraphicsItem::ItemClipsChildrenToShape);
    EXPECT_EQ(branch->parentItem(), scene.itemAt({1, 2}));
  }

  TEST(GraphScene, ANodeOverTheHeaderIsClippedAwayThere) {
    // Constant 10 starts two units above the body, under the header band.
    GraphScene scene{kCtx};
    scene.reconcile(functionWith({makeConstant(10, {.x = 2, .y = -2, .z = 0, .width = 10, .height = 10})}));
    ASSERT_NE(scene.itemAt({1, 10}), nullptr);
    const Rect node = toRect(scene.itemAt({1, 10})->sceneBoundingRect());
    const Vec2 inHeader{node.x + node.w / 2, node.y + 1};
    const Vec2 inBody{node.x + node.w / 2, node.y + node.h - 1};

    const QList<QGraphicsItem*> header = scene.items(QPointF(inHeader.x, inHeader.y));
    EXPECT_FALSE(header.contains(scene.itemAt({1, 10})));
    EXPECT_TRUE(scene.items(QPointF(inBody.x, inBody.y)).contains(scene.itemAt({1, 10})));
  }

  TEST(GraphScene, TheFrameChromePaintsOverTheBody) {
    GraphScene scene{kCtx};
    scene.reconcile(functionWith({}));
    ASSERT_NE(scene.itemAt({1}), nullptr);
    const Rect frame = toRect(scene.itemAt({1})->sceneBoundingRect());

    const QGraphicsItem* top = topAt(scene, Vec2{frame.x + frame.w / 2, frame.y + 1});

    ASSERT_NE(top, nullptr);
    EXPECT_NE(top, scene.itemAt({1}));
    EXPECT_EQ(blockFor(top), scene.itemAt({1}));
  }

  TEST(GraphScene, TheHigherZNodePaintsOnTop) {
    const auto overlapping = [](int z10, int z11) {
      return functionWith({makeConstant(10, {.x = 1, .y = 1, .z = z10, .width = 10, .height = 10}),
                           makeConstant(11, {.x = 2, .y = 2, .z = z11, .width = 10, .height = 10})});
    };
    GraphScene scene{kCtx};
    scene.reconcile(overlapping(1, 0));
    ASSERT_NE(scene.itemAt({1, 11}), nullptr);
    const QPointF overlap = scene.itemAt({1, 11})->sceneBoundingRect().center();
    const auto topNode = [&] {
      for (const QGraphicsItem* item : scene.items(overlap)) {
        if (const auto* block = dynamic_cast<const Element*>(item); block && block->modelPath().size() == 2) {
          return block->modelPath();
        }
      }
      return FullID{};
    };
    EXPECT_EQ(topNode(), (FullID{1, 10}));

    scene.reconcile(overlapping(0, 1));

    EXPECT_EQ(topNode(), (FullID{1, 11}));
  }

  TEST(GraphScene, TopLevelDeclarationsInterleaveByZ) {
    et::ParseTree tree = functionWith({});
    et::Comment comment{.id = 2, .location = {.x = 10, .y = 10, .z = 5, .width = 20, .height = 20}, .text = "hi"};
    tree.declarations.emplace(2, et::Declaration{comment});
    GraphScene scene{kCtx};
    scene.reconcile(tree);
    ASSERT_NE(scene.itemAt({2}), nullptr);
    const QPointF overlap = scene.itemAt({2})->sceneBoundingRect().center();

    EXPECT_EQ(blockFor(topAt(scene, toVec(overlap))), scene.itemAt({2}));
  }

  TEST(GraphScene, AConditionalHeaderCarriesItsBranchArrow) {
    GraphScene scene{kCtx};
    scene.reconcile(loadTree("read/conditional_with_body.fl"));
    ASSERT_NE(scene.itemAt({1, 2}), nullptr);

    EXPECT_EQ(componentsOf<fluir::editor::BranchArrow>(scene.itemAt({1, 2})).size(), 1u);
    EXPECT_EQ(componentsOf<fluir::editor::Frame>(scene.itemAt({1, 2})).size(), 1u);
  }

}  // namespace
