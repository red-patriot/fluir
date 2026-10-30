#include "editor/diagram/build.hpp"

#include <concepts>
#include <filesystem>
#include <memory>
#include <QGraphicsItem>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/editor_context.hpp"
#include "editor/diagram/binary.hpp"
#include "editor/diagram/call.hpp"
#include "editor/diagram/comment.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/node.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/diagram/unary.hpp"
#include "editor/scene/module_scene.hpp"
#include "fixture_loader.hpp"

namespace {

  namespace fs = std::filesystem;
  namespace diagram = fluir::editor::diagram;

  struct Built {
    fluir::editor::EditorContext ctx;
    fluir::editor::ModuleScene scene{ctx};
    bool ok = false;
  };

  // A scene is not movable, so it lives behind a pointer.
  std::unique_ptr<Built> build(const fs::path& fixture) {
    auto built = std::make_unique<Built>();
    const testutil::Loaded loaded = testutil::loadFixture(fixture.string());
    EXPECT_TRUE(loaded.result.tree.has_value()) << fixture;
    if (loaded.result.tree) {
      built->ok = diagram::buildScene(*loaded.result.tree, built->scene);
    }
    return built;
  }

  template <typename T>
  std::vector<T*> all(const QGraphicsScene& scene) {
    std::vector<T*> out;
    for (QGraphicsItem* item : scene.items()) {
      if (auto* t = dynamic_cast<T*>(item)) {
        out.push_back(t);
      }
    }
    return out;
  }

  bool isNode(const QGraphicsItem* item) { return dynamic_cast<const diagram::FluirNode*>(item) != nullptr; }

  // The function scope a node lives in, null if at the top level.
  const diagram::Scope* scopeOf(const QGraphicsItem* item) {
    for (const QGraphicsItem* up = item->parentItem(); up != nullptr; up = up->parentItem()) {
      if (const auto* scope = dynamic_cast<const diagram::Scope*>(up)) {
        return scope;
      }
    }
    return nullptr;
  }

  bool near(double a, double b) { return std::abs(a - b) < 1e-6; }

  bool near(QPointF a, QPointF b) { return near(a.x(), b.x()) && near(a.y(), b.y()); }

  QPointF centerOf(const diagram::Terminal* terminal) { return terminal->sceneBoundingRect().center(); }

  // Whether `path` (in `item`'s coordinates) has a line from `from` to `to` (in scene coordinates).
  bool hasLine(const QGraphicsItem& item, const QPainterPath& path, QPointF from, QPointF to) {
    for (int i = 0; i + 1 < path.elementCount(); ++i) {
      const QPainterPath::Element a = path.elementAt(i);
      const QPainterPath::Element b = path.elementAt(i + 1);
      if (a.isMoveTo() && b.isLineTo() && near(item.mapToScene(QPointF(a)), from) &&
          near(item.mapToScene(QPointF(b)), to)) {
        return true;
      }
    }
    return false;
  }

}  // namespace

static_assert(std::derived_from<diagram::Constant, diagram::FluirNode>);
static_assert(std::derived_from<diagram::Binary, diagram::FluirNode>);
static_assert(std::derived_from<diagram::Unary, diagram::FluirNode>);
static_assert(std::derived_from<diagram::Call, diagram::FluirNode>);
static_assert(std::derived_from<diagram::Comment, diagram::FluirNode>);
static_assert(std::derived_from<diagram::Conditional, diagram::FluirNode>);
static_assert(std::derived_from<diagram::Function, diagram::FluirNode>);

class BuildEachFixture : public testing::TestWithParam<fs::path> { };

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         BuildEachFixture,
                         testing::ValuesIn(testutil::readWriteFixtures(true)),
                         testutil::fixtureName);

TEST_P(BuildEachFixture, Builds) { EXPECT_TRUE(build(GetParam())->ok); }

TEST_P(BuildEachFixture, NodesSitInsideTheirScope) {
  const auto built = build(GetParam());
  ASSERT_TRUE(built->ok);

  for (const QGraphicsItem* item : built->scene.items()) {
    const diagram::Scope* scope = isNode(item) ? scopeOf(item) : nullptr;
    if (scope != nullptr) {
      EXPECT_TRUE(scope->sceneBoundingRect().contains(item->sceneBoundingRect()))
        << "node at " << item->sceneBoundingRect().x() << "," << item->sceneBoundingRect().y();
    }
  }
}

TEST_P(BuildEachFixture, InputsSitOnTheLeftWallAndOutputsOnTheRight) {
  const auto built = build(GetParam());
  ASSERT_TRUE(built->ok);

  for (const diagram::Terminal* terminal : all<diagram::Terminal>(built->scene)) {
    ASSERT_NE(terminal->parentItem(), nullptr);
    const QRectF owner = terminal->parentItem()->sceneBoundingRect();
    const QPointF center = centerOf(terminal);
    EXPECT_DOUBLE_EQ(center.x(), terminal->isOutput() ? owner.right() : owner.left());
    EXPECT_GE(center.y(), owner.top());
    EXPECT_LE(center.y(), owner.bottom());
  }
}

TEST_P(BuildEachFixture, ConduitsRunFromTheirSourceToEachSink) {
  const auto built = build(GetParam());
  ASSERT_TRUE(built->ok);

  for (const diagram::Conduit* conduit : all<diagram::Conduit>(built->scene)) {
    ASSERT_NE(conduit->source(), nullptr);
    EXPECT_TRUE(conduit->source()->isOutput());
    ASSERT_FALSE(conduit->sinks().empty());
    for (const diagram::Terminal* sink : conduit->sinks()) {
      ASSERT_NE(sink, nullptr);
      EXPECT_FALSE(sink->isOutput());
      EXPECT_TRUE(hasLine(*conduit, conduit->path(), centerOf(conduit->source()), centerOf(sink)));
    }
  }
}

TEST_P(BuildEachFixture, ABranchFillsItsConditionalBelowTheHeader) {
  const auto built = build(GetParam());
  ASSERT_TRUE(built->ok);

  for (const diagram::Conditional* conditional : all<diagram::Conditional>(built->scene)) {
    const QRectF expected = conditional->sceneBoundingRect().adjusted(0, built->ctx.layout.headerH(), 0, 0);
    for (const diagram::Scope* branch : {conditional->thenBranch(), conditional->elseBranch()}) {
      ASSERT_NE(branch, nullptr);
      EXPECT_EQ(branch->parentItem(), conditional);
      EXPECT_EQ(branch->sceneBoundingRect(), expected);
    }
  }
}

TEST_P(BuildEachFixture, OnlyTheThenBranchShows) {
  const auto built = build(GetParam());
  ASSERT_TRUE(built->ok);

  for (const diagram::Conditional* conditional : all<diagram::Conditional>(built->scene)) {
    EXPECT_TRUE(conditional->thenBranch()->isVisible());
    EXPECT_FALSE(conditional->elseBranch()->isVisible());
  }
}

TEST(Build, TheHiddenBranchStillHoldsItsNodes) {
  // conditional_with_body.fl: the else branch holds three nodes and two conduits.
  const auto built = build(fs::path(TEST_FOLDER) / "read/conditional_with_body.fl");
  ASSERT_TRUE(built->ok);
  const std::vector<diagram::Conditional*> conditionals = all<diagram::Conditional>(built->scene);
  ASSERT_EQ(conditionals.size(), 1u);

  int nodes = 0;
  int conduits = 0;
  for (const QGraphicsItem* item : built->scene.items()) {
    if (scopeOf(item) == conditionals[0]->elseBranch()) {
      nodes += isNode(item) ? 1 : 0;
      conduits += dynamic_cast<const diagram::Conduit*>(item) != nullptr ? 1 : 0;
    }
  }
  EXPECT_EQ(nodes, 3);
  EXPECT_EQ(conduits, 2);
}

class BuildFails : public testing::TestWithParam<fs::path> { };

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         BuildFails,
                         testing::ValuesIn(testutil::fixturesIn("build_fails")),
                         testutil::fixtureName);

// These parse, but describe no diagram: a conduit end that resolves to no terminal of the right side, or two
// endpoints sharing an id in one scope.
TEST_P(BuildFails, RejectsAnInvalidDiagram) { EXPECT_FALSE(build(GetParam())->ok); }
