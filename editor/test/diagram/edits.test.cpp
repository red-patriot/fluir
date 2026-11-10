#include "editor/diagram/edits.hpp"

#include <QAction>
#include <QMenu>
#include <QPoint>
#include <QStringList>
#include <QTest>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/choices.hpp"
#include "editor/core/field.hpp"
#include "editor/diagram/binary.hpp"
#include "editor/diagram/edit_target.hpp"
#include "interaction.hpp"

// Edits work on any EditTarget, so any item can host them. A FieldTarget edits one field of a node through SetField.

namespace {

  namespace diagram = fluir::editor::diagram;
  using fluir::editor::Field;
  using testutil::Interaction;

  constexpr Field kOperator{Field::Kind::Operator};

  // A target that offers `offered` and records what is committed to it.
  struct Recorder : diagram::EditTarget {
    std::string value;
    std::vector<std::string> offered;
    std::vector<std::string> commits;

    std::string current() const override { return value; }
    bool validate(std::string_view) const override { return true; }
    std::vector<std::string> options() const override { return offered; }
    void commit(std::string_view text) override { commits.emplace_back(text); }
  };

  QStringList entries(const QMenu& menu) {
    QStringList out;
    for (const QAction* action : menu.actions()) {
      out << action->text();
    }
    return out;
  }

}  // namespace

TEST(Pick, OffersTheTargetsOptionsAndCommitsThePickedOne) {
  Interaction h("read/simple_binary_expr.fl");
  Recorder target;
  target.value = "a";
  target.offered = {"a", "b"};

  diagram::pick(target, h.view().mapToGlobal(QPoint{}));
  QMenu* menu = Interaction::popup();
  ASSERT_NE(menu, nullptr);
  EXPECT_EQ(entries(*menu), (QStringList{"a", "b"}));
  menu->actions().back()->trigger();
  if (QMenu* open = Interaction::popup()) {
    open->close();
  }

  EXPECT_EQ(target.commits, std::vector<std::string>{"b"});
}

TEST(Pick, DismissingCommitsNothing) {
  Interaction h("read/simple_binary_expr.fl");
  Recorder target;
  target.offered = {"a", "b"};

  diagram::pick(target, h.view().mapToGlobal(QPoint{}));
  ASSERT_NE(Interaction::popup(), nullptr);
  QTest::keyClick(Interaction::popup(), Qt::Key_Escape);

  EXPECT_EQ(Interaction::popup(), nullptr);
  EXPECT_TRUE(target.commits.empty());
}

TEST(Flip, CommitsTheOppositeBool) {
  Recorder off;
  off.value = "false";
  Recorder on;
  on.value = "true";

  diagram::flip(off);
  diagram::flip(on);

  EXPECT_EQ(off.commits, std::vector<std::string>{"true"});
  EXPECT_EQ(on.commits, std::vector<std::string>{"false"});
}

TEST(FieldTarget, ReadsValidatesAndOffersItsNodesField) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const diagram::FieldTarget target(binary, [] { return kOperator; });

  EXPECT_EQ(target.current(), binary.text(kOperator));
  EXPECT_EQ(target.options(), fluir::editor::binaryOperators());
  EXPECT_TRUE(target.validate("*"));
  EXPECT_FALSE(target.validate("not an operator"));
}

TEST(FieldTarget, CommitPushesOneUndoableSetFieldAndNothingForTheSameValue) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  diagram::FieldTarget target(binary, [] { return kOperator; });
  const fluir::pt::ParseTree before = h.tree();

  target.commit(target.current());
  EXPECT_EQ(h.stack().count(), 0);

  target.commit("*");
  EXPECT_EQ(binary.text(kOperator), "*");
  ASSERT_EQ(h.stack().count(), 1);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
}
