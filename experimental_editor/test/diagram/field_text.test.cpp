#include "editor/diagram/field_text.hpp"

#include <QPointF>
#include <QTest>

#include <gtest/gtest.h>

#include "editor/core/field.hpp"
#include "editor/diagram/call.hpp"
#include "editor/diagram/comment.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "interaction.hpp"

// A press opens a text field and selects its node. Enter commits a valid draft as one command, Escape discards it,
// and leaving commits it if valid. The border shows an invalid draft while it is typed.
// int_constants.fl: constant 1 is the I8 -5.

namespace {

  namespace diagram = fluir::editor::diagram;
  using fluir::editor::Field;
  using testutil::Interaction;

  constexpr Field kLiteral{Field::Kind::Literal};

  struct Harness : Interaction {
    Harness() : Interaction("read/int_constants.fl") { }

    diagram::Constant& constant() { return find<diagram::Constant>(1); }
    diagram::FieldText& field() { return fieldOf<diagram::FieldText>(constant(), kLiteral); }
    void clickAway() { click(aimAt(find<diagram::Function>(1).sceneBoundingRect().topLeft() - QPointF(20, 20))); }
  };

}  // namespace

TEST(FieldText, APressOpensItAndSelectsItsNode) {
  Harness h;

  h.click(h.aim(h.field()));

  EXPECT_TRUE(h.field().hasFocus());
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&h.constant()});
}

TEST(FieldText, EnterCommitsOneUndoableEdit) {
  Harness h;
  const fluir::pt::ParseTree before = h.tree();

  h.typeInto(h.field(), "42");
  h.key(Qt::Key_Return);

  EXPECT_FALSE(h.field().hasFocus());
  EXPECT_EQ(h.constant().text(kLiteral), "42");
  ASSERT_EQ(h.stack().count(), 1);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.field().toPlainText(), "-5");
  h.stack().redo();
  EXPECT_EQ(h.field().toPlainText(), "42");
}

TEST(FieldText, EscapeDiscardsTheDraft) {
  Harness h;

  h.typeInto(h.field(), "42");
  h.key(Qt::Key_Escape);

  EXPECT_FALSE(h.field().hasFocus());
  EXPECT_EQ(h.field().toPlainText(), "-5");
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(FieldText, LeavingCommitsAValidDraft) {
  Harness h;

  h.typeInto(h.field(), "42");
  h.clickAway();

  EXPECT_EQ(h.constant().text(kLiteral), "42");
  EXPECT_EQ(h.stack().count(), 1);
}

TEST(FieldText, LeavingDiscardsAnInvalidDraft) {
  Harness h;

  h.typeInto(h.field(), "abc");
  h.clickAway();

  EXPECT_EQ(h.constant().text(kLiteral), "-5");
  EXPECT_EQ(h.field().toPlainText(), "-5");
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(FieldText, AnInvalidDraftShowsWhileTypedAndEnterKeepsItOpen) {
  Harness h;

  h.typeInto(h.field(), "4");
  EXPECT_TRUE(h.field().isValid());
  QTest::keyClicks(&h.view(), "x");
  EXPECT_FALSE(h.field().isValid());
  h.key(Qt::Key_Return);

  EXPECT_TRUE(h.field().hasFocus());
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(FieldText, CommittingTheUnchangedValuePushesNothing) {
  Harness h;

  h.typeInto(h.field(), "-5");
  h.key(Qt::Key_Return);

  EXPECT_EQ(h.stack().count(), 0);
}

TEST(FieldText, ShiftEnterCommitsASingleLineField) {
  Harness h;

  h.typeInto(h.field(), "4");
  h.key(Qt::Key_Return, Qt::ShiftModifier);

  EXPECT_EQ(h.constant().text(kLiteral), "4");
}

TEST(FieldText, ShiftEnterBreaksALineInAComment) {
  Interaction h("read/top_level_comment_only.fl");
  auto& comment = h.find<diagram::Comment>(1);
  auto& field = Interaction::fieldOf<diagram::FieldText>(comment, {Field::Kind::Text});

  h.typeInto(field, "a");
  h.key(Qt::Key_Return, Qt::ShiftModifier);
  QTest::keyClicks(&h.view(), "b");
  h.key(Qt::Key_Return);

  EXPECT_EQ(comment.text({Field::Kind::Text}), "a\nb");
  EXPECT_EQ(h.stack().count(), 1);
}

TEST(FieldText, AFunctionNameRenames) {
  Interaction h("read/simple_binary_expr.fl");
  auto& function = h.find<diagram::Function>(1);

  h.typeInto(Interaction::fieldOf<diagram::FieldText>(function, {Field::Kind::Name}), "renamed");
  h.key(Qt::Key_Return);

  EXPECT_EQ(function.text({Field::Kind::Name}), "renamed");
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&function});
}

TEST(FieldText, AParameterNameRenames) {
  Interaction h("read/function_with_input_only.fl");
  auto& function = h.find<diagram::Function>(1);
  const Field name{Field::Kind::ParamName, 0};

  h.typeInto(Interaction::fieldOf<diagram::FieldText>(function, name), "x");
  h.key(Qt::Key_Return);

  EXPECT_EQ(function.text(name), "x");
}

TEST(FieldText, ACallArgumentRenames) {
  Interaction h("read/function_call.fl");
  auto& call = h.find<diagram::Call>(3);
  const Field second{Field::Kind::Arg, 1};

  h.typeInto(Interaction::fieldOf<diagram::FieldText>(call, second), "rhs");
  h.key(Qt::Key_Return);

  EXPECT_EQ(call.text(second), "rhs");
  EXPECT_EQ(call.text({Field::Kind::Arg, 0}), "a");
}
