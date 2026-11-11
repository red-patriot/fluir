#include "editor/diagram/selection_outline.hpp"

#include <QRectF>

#include <gtest/gtest.h>

#include "compiler/models/location.hpp"
#include "editor/diagram/constant.hpp"
#include "interaction.hpp"

namespace {

  namespace diagram = fluir::editor::diagram;
  using diagram::SelectionOutline;
  using testutil::Interaction;

  bool surrounds(const SelectionOutline& outline, const diagram::FluirNode& node) {
    const QRectF out = outline.sceneBoundingRect();
    const QRectF in = node.sceneBoundingRect();
    return out.contains(in) && out != in;
  }

}  // namespace

TEST(SelectionOutline, ShowsOnlyWhileItsNodeIsSelected) {
  Interaction h("read/simple_binary_expr.fl");
  auto& first = h.find<diagram::Constant>(2);
  auto& second = h.find<diagram::Constant>(3);
  const auto& outline = Interaction::part<SelectionOutline>(first);
  EXPECT_FALSE(outline.isVisible());

  h.click(h.body(first));
  EXPECT_TRUE(outline.isVisible());

  h.click(h.body(second));
  EXPECT_FALSE(outline.isVisible());
}

TEST(SelectionOutline, SurroundsItsNode) {
  Interaction h("read/simple_binary_expr.fl");
  auto& node = h.find<diagram::Constant>(2);

  EXPECT_TRUE(surrounds(Interaction::part<SelectionOutline>(node), node));
}

TEST(SelectionOutline, FollowsAResize) {
  Interaction h("read/simple_binary_expr.fl");
  auto& node = h.find<diagram::Constant>(2);
  fluir::FlowGraphLocation bigger = node.location();
  bigger.width += 2;
  bigger.height += 1;

  node.setLocation(bigger);

  EXPECT_TRUE(surrounds(Interaction::part<SelectionOutline>(node), node));
}
