#include <QAbstractItemModel>
#include <QApplication>
#include <QLineEdit>
#include <QListView>
#include <QPointF>
#include <QStringList>
#include <QTest>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/commands/add_node.hpp"
#include "editor/core/intelligence.hpp"
#include "editor/core/scope_path.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/function.hpp"
#include "editor/scene/completion_popup.hpp"
#include "interaction.hpp"

// A right-click on empty canvas, a function body or a visible branch pops up what Intelligence offers there. Picking an
// option pushes one AddNode at the clicked grid point, the same as pushing it directly, and selects nothing.
// fan_out.fl: function 1 at (10, 10), 100 x 100. ported_conditional.fl: function 1 holds conditional 2.

namespace {

  namespace diagram = fluir::editor::diagram;
  namespace pt = fluir::pt;
  using fluir::editor::AddNode;
  using fluir::editor::Completion;
  using fluir::editor::Intelligence;
  using fluir::editor::Vec2i;
  using testutil::Interaction;

  // Clear of fan_out's nodes and conduits, and of ported_conditional's then branch contents.
  constexpr Vec2i kInBody{60, 60};
  constexpr Vec2i kInBranch{70, 60};
  // Right of fan_out's function.
  constexpr Vec2i kOnCanvas{150, 20};

  fluir::editor::CompletionPopup* completions() {
    return dynamic_cast<fluir::editor::CompletionPopup*>(QApplication::activePopupWidget());
  }

  QStringList shown() {
    QStringList out;
    const QAbstractItemModel& model = *completions()->findChild<QListView*>()->model();
    for (int row = 0; row < model.rowCount(); ++row) {
      out << model.index(row, 0).data().toString();
    }
    return out;
  }

  template <typename T>
  QStringList labelsOf(const std::vector<Completion<T>>& options) {
    QStringList out;
    for (const Completion<T>& option : options) {
      out << QString::fromStdString(option.label);
    }
    return out;
  }

  template <typename T>
  T prototypeOf(const std::vector<Completion<T>>& options, const std::string& label) {
    for (const Completion<T>& option : options) {
      if (option.label == label) {
        return option.prototype;
      }
    }
    throw std::logic_error("no such completion");
  }

  // Right-clicks grid point `at` of `scope`.
  void contextClickIn(Interaction& h, const diagram::Scope& scope, Vec2i at) {
    h.contextClick(h.aimAt(scope.mapToScene(QPointF(at.x * h.unit(), at.y * h.unit()))));
  }

  void contextClickOnCanvas(Interaction& h, Vec2i at) {
    h.contextClick(h.aimAt(QPointF(at.x * h.unit(), at.y * h.unit())));
  }

  // Types `label` into the open popup and picks the top match.
  void pick(const std::string& label) {
    ASSERT_NE(completions(), nullptr);
    QLineEdit& filter = *completions()->findChild<QLineEdit*>();
    QTest::keyClicks(&filter, QString::fromStdString(label));
    QTest::keyClick(&filter, Qt::Key_Return);
  }

  struct Closer {
    ~Closer() {
      if (QWidget* open = completions()) {
        open->close();
      }
    }
  };

}  // namespace

TEST(Completion, AFunctionBodyOffersItsCompletions) {
  Closer closer;
  Interaction h("interaction/fan_out.fl");

  contextClickIn(h, *h.find<diagram::Function>(1).body(), kInBody);

  ASSERT_NE(completions(), nullptr);
  EXPECT_EQ(shown(), labelsOf(Intelligence{}.completionsAt(h.tree(), {1})));
}

TEST(Completion, PickingInABodyAddsAtTheClickPoint) {
  Closer closer;
  pt::ParseTree expected;
  {
    Interaction twin("interaction/fan_out.fl");
    const pt::Node comment = prototypeOf(Intelligence{}.completionsAt(twin.tree(), {1}), "Comment");
    twin.stack().push(new AddNode(*twin.find<diagram::Function>(1).body(), comment, kInBody));
    expected = twin.tree();
  }
  Interaction h("interaction/fan_out.fl");

  contextClickIn(h, *h.find<diagram::Function>(1).body(), kInBody);
  pick("Comment");

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(h.stack().count(), 1);
  EXPECT_TRUE(h.scene().selectedItems().isEmpty());
  EXPECT_EQ(completions(), nullptr);
}

TEST(Completion, PickingInAVisibleBranchAddsToThatBranch) {
  Closer closer;
  const fluir::FullID then{1, 2, fluir::editor::THEN_BRANCH_ID};
  pt::ParseTree expected;
  {
    Interaction twin("interaction/ported_conditional.fl");
    const pt::Node comment = prototypeOf(Intelligence{}.completionsAt(twin.tree(), then), "Comment");
    twin.stack().push(new AddNode(*twin.find<diagram::Conditional>(2).thenBranch(), comment, kInBranch));
    expected = twin.tree();
  }
  Interaction h("interaction/ported_conditional.fl");
  const diagram::Scope& branch = *h.find<diagram::Conditional>(2).thenBranch();
  ASSERT_TRUE(branch.isVisible());

  contextClickIn(h, branch, kInBranch);
  ASSERT_NE(completions(), nullptr);
  EXPECT_EQ(shown(), labelsOf(Intelligence{}.completionsAt(h.tree(), then)));
  pick("Comment");

  EXPECT_EQ(h.tree(), expected);
}

TEST(Completion, PickingOnTheCanvasAddsAtTheTopLevel) {
  Closer closer;
  pt::ParseTree expected;
  {
    Interaction twin("interaction/fan_out.fl");
    const pt::Declaration function = prototypeOf(Intelligence{}.topLevelCompletions(twin.tree()), "Function");
    twin.stack().push(new AddNode(twin.scene(), function, kOnCanvas));
    expected = twin.tree();
  }
  Interaction h("interaction/fan_out.fl");

  contextClickOnCanvas(h, kOnCanvas);
  ASSERT_NE(completions(), nullptr);
  EXPECT_EQ(shown(), labelsOf(Intelligence{}.topLevelCompletions(h.tree())));
  pick("Function");

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(h.stack().count(), 1);
}

TEST(Completion, EscapeAddsNothing) {
  Closer closer;
  Interaction h("interaction/fan_out.fl");
  const pt::ParseTree before = h.tree();

  contextClickIn(h, *h.find<diagram::Function>(1).body(), kInBody);
  ASSERT_NE(completions(), nullptr);
  QTest::keyClick(completions()->findChild<QLineEdit*>(), Qt::Key_Escape);

  EXPECT_EQ(completions(), nullptr);
  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}
