#include "editor/diagram/field_view.hpp"

#include <QAction>
#include <QMenu>
#include <QStringList>
#include <QTest>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/choices.hpp"
#include "editor/core/field.hpp"
#include "editor/diagram/binary.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/unary.hpp"
#include "editor/scene/item_style.hpp"
#include "interaction.hpp"

// A press on a menu-picked field view selects its node and pops a menu of its options. Picking a different option
// pushes one command; picking the current one or dismissing the menu changes nothing.

namespace {

  namespace diagram = fluir::editor::diagram;
  using fluir::editor::Field;
  using testutil::Interaction;

  constexpr Field kOperator{Field::Kind::Operator};

  QStringList entries(const QMenu& menu) {
    QStringList out;
    for (const QAction* action : menu.actions()) {
      out << action->text();
    }
    return out;
  }

  QStringList listOf(const std::vector<std::string>& options) {
    QStringList out;
    for (const std::string& option : options) {
      out << QString::fromStdString(option);
    }
    return out;
  }

  // The operator's text, left of the move grip that covers the node's center.
  QPoint operatorOf(Interaction& h, const diagram::FluirNode& node) {
    const auto& choice = Interaction::fieldOf<diagram::FieldView>(node, kOperator);
    return h.aim(choice, QPointF(-choice.sceneBoundingRect().width() / 4, 0));
  }

  // Picks `entry` from the open menu.
  void pick(const QString& entry) {
    QMenu* menu = Interaction::popup();
    ASSERT_NE(menu, nullptr);
    for (QAction* action : menu->actions()) {
      if (action->text() == entry) {
        action->trigger();
      }
    }
    if (QMenu* open = Interaction::popup()) {
      open->close();
    }
  }

}  // namespace

TEST(FieldView, APressOnABinaryOffersItsOperatorsAndSelectsIt) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);

  h.click(operatorOf(h, binary));

  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), listOf(fluir::editor::binaryOperators()));
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&binary});
}

TEST(FieldView, PickingAnotherOptionPushesOneUndoableEdit) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const fluir::pt::ParseTree before = h.tree();

  h.click(operatorOf(h, binary));
  pick("*");

  EXPECT_EQ(binary.text(kOperator), "*");
  ASSERT_EQ(h.stack().count(), 1);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(Interaction::fieldOf<diagram::FieldView>(binary, kOperator).text(), "+");
}

TEST(FieldView, PickingTheCurrentOptionPushesNothing) {
  Interaction h("read/simple_binary_expr.fl");

  h.click(operatorOf(h, h.find<diagram::Binary>(1)));
  pick("+");

  EXPECT_EQ(h.stack().count(), 0);
}

TEST(FieldView, DismissingTheMenuChangesNothing) {
  Interaction h("read/simple_binary_expr.fl");
  const fluir::pt::ParseTree before = h.tree();

  h.click(operatorOf(h, h.find<diagram::Binary>(1)));
  ASSERT_NE(Interaction::popup(), nullptr);
  QTest::keyClick(Interaction::popup(), Qt::Key_Escape);

  EXPECT_EQ(Interaction::popup(), nullptr);
  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}

TEST(FieldView, AUnaryOffersItsOperators) {
  Interaction h("read/simple_unary_expr.fl");

  h.click(operatorOf(h, h.find<diagram::Unary>(7)));

  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), listOf(fluir::editor::unaryOperators()));
  Interaction::popup()->close();
}

TEST(FieldView, AParameterTypeOffersTheBuiltinTypes) {
  Interaction h("read/function_with_input_only.fl");
  auto& function = h.find<diagram::Function>(1);
  const Field type{Field::Kind::ParamType, 1};

  h.click(h.aim(Interaction::fieldOf<diagram::FieldView>(function, type)));
  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), listOf(fluir::editor::builtinTypes()));
  pick("U8");

  EXPECT_EQ(function.text(type), "U8");
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&function});
}

TEST(FieldView, AReturnTypeOffersTheBuiltinTypes) {
  Interaction h("read/function_with_output_only.fl");
  auto& function = h.find<diagram::Function>(1);

  h.click(h.aim(Interaction::fieldOf<diagram::FieldView>(function, {Field::Kind::ReturnType})));
  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), listOf(fluir::editor::builtinTypes()));
  pick("BOOL");

  EXPECT_EQ(function.text({Field::Kind::ReturnType}), "BOOL");
}

TEST(FieldView, ATypeShowsInTheTagFontAndAnOperatorInTheUiFont) {
  Interaction in("read/function_with_input_only.fl");
  Interaction out("read/function_with_output_only.fl");
  Interaction op("read/simple_binary_expr.fl");
  namespace style = fluir::editor::style;

  EXPECT_EQ(Interaction::fieldOf<diagram::FieldView>(in.find<diagram::Function>(1), {Field::Kind::ParamType, 0}).font(),
            style::tagFont());
  EXPECT_EQ(Interaction::fieldOf<diagram::FieldView>(out.find<diagram::Function>(1), {Field::Kind::ReturnType}).font(),
            style::tagFont());
  EXPECT_EQ(Interaction::fieldOf<diagram::FieldView>(op.find<diagram::Binary>(1), kOperator).font(), style::uiFont());
}
