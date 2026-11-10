#include <QAction>
#include <QMenu>
#include <QPoint>
#include <QPointF>
#include <QStringList>
#include <QTest>
#include <QUndoCommand>
#include <stdexcept>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/commands/add_parameter.hpp"
#include "editor/commands/add_port.hpp"
#include "editor/commands/add_return.hpp"
#include "editor/commands/delete_parameter.hpp"
#include "editor/commands/delete_port.hpp"
#include "editor/commands/delete_return.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/move_grip.hpp"
#include "editor/diagram/parameter.hpp"
#include "editor/diagram/port.hpp"
#include "editor/diagram/return.hpp"
#include "editor/view/draw/conditional.hpp"
#include "interaction.hpp"

// A right-click selects what it lands on and pops that thing's menu. Each entry pushes the same edit as its command.

namespace {

  namespace diagram = fluir::editor::diagram;
  namespace draw = fluir::editor::draw;
  namespace pt = fluir::pt;
  using testutil::Interaction;

  constexpr draw::PortRef kCondition{};
  constexpr draw::PortRef kFirstInput{.output = false, .index = 1};

  QStringList entries(const QMenu& menu) {
    QStringList out;
    for (const QAction* action : menu.actions()) {
      if (!action->isSeparator()) {
        out << action->text();
      }
    }
    return out;
  }

  QAction* entry(const QString& text) {
    QMenu* menu = Interaction::popup();
    if (menu == nullptr) {
      throw std::logic_error("no menu open");
    }
    for (QAction* action : menu->actions()) {
      if (action->text() == text) {
        return action;
      }
    }
    throw std::logic_error("no such entry");
  }

  // Triggers `text` in the open menu, then closes it.
  void pick(const QString& text) {
    entry(text)->trigger();
    if (QMenu* open = Interaction::popup()) {
      open->close();
    }
  }

  // The tree a fresh page over `fixture` exports after `make(page)` is pushed.
  template <typename Make>
  pt::ParseTree afterPushing(const std::string& fixture, Make make) {
    Interaction twin{fixture};
    twin.stack().push(make(twin));
    return twin.tree();
  }

  // A point in `container`'s header, a quarter of the way in from its left.
  QPoint headerOf(Interaction& h, const QGraphicsItem& container) {
    const QRectF frame = container.sceneBoundingRect();
    return h.aim(container, QPointF(-frame.width() / 4, h.ctx().layout.headerH() / 2 - frame.height() / 2));
  }

  diagram::Port& portOf(diagram::Conditional& conditional, draw::PortRef ref) {
    for (diagram::Port* port : conditional.wallPorts()) {
      if (port->ref() == ref) {
        return *port;
      }
    }
    throw std::logic_error("no such port");
  }

}  // namespace

TEST(ContextMenu, AFunctionsHeaderSelectsItAndOffersItsEdits) {
  Interaction h{"interaction/wired_function.fl"};
  auto& function = h.find<diagram::Function>(1);

  h.contextClick(headerOf(h, function));

  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), (QStringList{"Add parameter", "Add return", "Delete"}));
  EXPECT_FALSE(entry("Add return")->isEnabled());
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&function});
}

TEST(ContextMenu, AFunctionsMoveGripOpensTheSameMenu) {
  Interaction h{"interaction/wired_function.fl"};

  h.contextClick(h.aim(Interaction::part<diagram::MoveGrip>(h.find<diagram::Function>(1))));

  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), (QStringList{"Add parameter", "Add return", "Delete"}));
}

TEST(ContextMenu, AddParameterPushesOneAddParameter) {
  const pt::ParseTree expected = afterPushing("interaction/wired_function.fl", [](Interaction& twin) {
    return new fluir::editor::AddParameter(twin.find<diagram::Function>(1));
  });
  Interaction h{"interaction/wired_function.fl"};

  h.contextClick(headerOf(h, h.find<diagram::Function>(1)));
  pick("Add parameter");

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(h.stack().count(), 1);
}

TEST(ContextMenu, AddReturnPushesAddReturnOnAFunctionWithoutOne) {
  const pt::ParseTree expected = afterPushing("read/function_with_input_only.fl", [](Interaction& twin) {
    return new fluir::editor::AddReturn(twin.find<diagram::Function>(1));
  });
  Interaction h{"read/function_with_input_only.fl"};

  h.contextClick(headerOf(h, h.find<diagram::Function>(1)));
  ASSERT_TRUE(entry("Add return")->isEnabled());
  pick("Add return");

  EXPECT_EQ(h.tree(), expected);
}

TEST(ContextMenu, AConditionalsHeaderSelectsItAndOffersItsEdits) {
  Interaction h{"interaction/ported_conditional.fl"};
  auto& conditional = h.find<diagram::Conditional>(2);

  h.contextClick(headerOf(h, conditional));

  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), (QStringList{"Add input", "Add output", "Delete"}));
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&conditional});
}

TEST(ContextMenu, AddInputAndAddOutputPushAddPort) {
  for (const bool output : {false, true}) {
    const pt::ParseTree expected = afterPushing("interaction/ported_conditional.fl", [output](Interaction& twin) {
      return new fluir::editor::AddPort(twin.find<diagram::Conditional>(2), output);
    });
    Interaction h{"interaction/ported_conditional.fl"};

    h.contextClick(headerOf(h, h.find<diagram::Conditional>(2)));
    pick(output ? "Add output" : "Add input");

    EXPECT_EQ(h.tree(), expected) << (output ? "output" : "input");
  }
}

TEST(ContextMenu, DeletePortPushesDeletePort) {
  const pt::ParseTree expected = afterPushing("interaction/ported_conditional.fl", [](Interaction& twin) {
    return new fluir::editor::DeletePort(twin.find<diagram::Conditional>(2), kFirstInput);
  });
  Interaction h{"interaction/ported_conditional.fl"};
  auto& conditional = h.find<diagram::Conditional>(2);

  h.contextClick(h.aim(portOf(conditional, kFirstInput)));
  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), QStringList{"Delete port"});
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&conditional});
  pick("Delete port");

  EXPECT_EQ(h.tree(), expected);
}

TEST(ContextMenu, TheConditionPortOffersNoMenu) {
  Interaction h{"interaction/ported_conditional.fl"};
  const pt::ParseTree before = h.tree();

  h.contextClick(h.aim(portOf(h.find<diagram::Conditional>(2), kCondition)));

  EXPECT_EQ(Interaction::popup(), nullptr);
  EXPECT_EQ(h.tree(), before);
}

TEST(ContextMenu, DeleteOnAParameterPushesDeleteParameter) {
  const pt::ParseTree expected = afterPushing("interaction/wired_function.fl", [](Interaction& twin) {
    return new fluir::editor::DeleteParameter(twin.find<diagram::Function>(1), 2);
  });
  Interaction h{"interaction/wired_function.fl"};

  h.contextClick(h.aim(h.find<diagram::Parameter>(2)));
  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), QStringList{"Delete"});
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&h.find<diagram::Function>(1)});
  pick("Delete");

  EXPECT_EQ(h.tree(), expected);
}

TEST(ContextMenu, DeleteOnAReturnPushesDeleteReturn) {
  const pt::ParseTree expected = afterPushing("interaction/wired_function.fl", [](Interaction& twin) {
    return new fluir::editor::DeleteReturn(twin.find<diagram::Function>(1));
  });
  Interaction h{"interaction/wired_function.fl"};

  h.contextClick(h.aim(h.find<diagram::Return>(9)));
  pick("Delete");

  EXPECT_EQ(h.tree(), expected);
}

TEST(ContextMenu, DeleteOnANodeMatchesTheDeleteKey) {
  pt::ParseTree expected;
  {
    Interaction twin{"interaction/fan_out.fl"};
    twin.select(twin.find<diagram::Constant>(1));
    twin.key(Qt::Key_Delete);
    expected = twin.tree();
  }
  Interaction h{"interaction/fan_out.fl"};
  auto& constant = h.find<diagram::Constant>(1);

  h.contextClick(h.body(constant));
  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), QStringList{"Delete"});
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&constant});
  pick("Delete");

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(h.stack().count(), 1);
}

TEST(ContextMenu, DeleteOnAConduitRemovesIt) {
  Interaction h{"interaction/fan_out.fl"};
  pt::ParseTree expected = h.tree();
  std::get<pt::FunctionDecl>(expected.declarations.at(1)).body.conduits.erase(6);
  auto& conduit = h.find<diagram::Conduit>(6);

  h.contextClick(h.aim(conduit));
  ASSERT_NE(Interaction::popup(), nullptr);
  EXPECT_EQ(entries(*Interaction::popup()), QStringList{"Delete"});
  EXPECT_EQ(h.scene().selectedItems(), QList<QGraphicsItem*>{&conduit});
  pick("Delete");

  EXPECT_EQ(h.tree(), expected);
}

TEST(ContextMenu, DismissingTheMenuChangesNothing) {
  Interaction h{"interaction/fan_out.fl"};
  const pt::ParseTree before = h.tree();

  h.contextClick(h.body(h.find<diagram::Constant>(1)));
  ASSERT_NE(Interaction::popup(), nullptr);
  QTest::keyClick(Interaction::popup(), Qt::Key_Escape);

  EXPECT_EQ(Interaction::popup(), nullptr);
  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(h.stack().count(), 0);
}
