#include "editor/diagram/toggle.hpp"

#include <gtest/gtest.h>

#include "editor/core/field.hpp"
#include "editor/diagram/constant.hpp"
#include "interaction.hpp"

// A press on a bool constant's toggle flips its value as one command and selects the constant.
// read_boolean.fl: constant 1 is false, constant 2 is true.

namespace {

  namespace diagram = fluir::editor::diagram;
  using fluir::editor::Field;
  using testutil::Interaction;

  constexpr Field kBool{Field::Kind::Bool};

}  // namespace

TEST(Toggle, APressFlipsTheValueAsOneUndoableEdit) {
  Interaction h("read/read_boolean.fl");
  auto& constant = h.find<diagram::Constant>(1);
  auto& toggle = Interaction::fieldOf<diagram::Toggle>(constant, kBool);
  const fluir::pt::ParseTree before = h.tree();

  h.click(h.aim(toggle));

  EXPECT_TRUE(toggle.value());
  EXPECT_EQ(constant.text(kBool), "true");
  ASSERT_EQ(h.stack().count(), 1);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_FALSE(toggle.value());
}

TEST(Toggle, APressSelectsItsConstant) {
  Interaction h("read/read_boolean.fl");
  auto& constant = h.find<diagram::Constant>(2);

  h.click(h.aim(Interaction::fieldOf<diagram::Toggle>(constant, kBool)));

  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&constant});
  EXPECT_EQ(constant.text(kBool), "false");
}
