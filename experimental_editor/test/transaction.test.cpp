#include "editor/transaction/transaction.hpp"

#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/transaction/move.hpp"
#include "editor/transaction/resize.hpp"
#include "editor/transaction/set_constant_value.hpp"

// The house round trip: apply, assert, reverse, assert the tree is back.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::DeleteTransaction;
  using fluir::editor::locationAt;
  using fluir::editor::MoveTransaction;
  using fluir::editor::ResizeTransaction;
  using fluir::editor::SetConstantValueTransaction;

  fluir::pt::Constant makeConstant(ID id, int x, int y) {
    return fluir::pt::Constant{.id = id,
                               .location = FlowGraphLocation{.x = x, .y = y, .z = 1, .width = 5, .height = 5},
                               .value = fluir::literals_types::I32{0}};
  }

  fluir::pt::Binary makeBinary(ID id, int x, int y, ID lhs, ID rhs) {
    return fluir::pt::Binary{.id = id,
                             .location = FlowGraphLocation{.x = x, .y = y, .z = 1, .width = 5, .height = 5},
                             .lhs = lhs,
                             .rhs = rhs,
                             .op = Operator::PLUS};
  }

  fluir::pt::Unary makeUnary(ID id, int x, int y, ID lhs) {
    return fluir::pt::Unary{.id = id,
                            .location = FlowGraphLocation{.x = x, .y = y, .z = 1, .width = 5, .height = 5},
                            .lhs = lhs,
                            .op = Operator::MINUS};
  }

  fluir::pt::Conduit makeConduit(ID id, ID input, std::vector<fluir::pt::Conduit::Output> children) {
    return fluir::pt::Conduit{.id = id, .input = input, .index = 0, .children = std::move(children)};
  }

  // Two constants feed binary 30, whose result feeds unary 31. Conduit 44 fans
  // out to both 30 and 31, so deleting 30 only strips one of its targets.
  fluir::pt::ParseTree makeTree() {
    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.body.nodes.emplace(10, makeConstant(10, 1, 1));
    fn.body.nodes.emplace(11, makeConstant(11, 1, 10));
    fn.body.nodes.emplace(30, makeBinary(30, 10, 1, 10, 11));
    fn.body.nodes.emplace(31, makeUnary(31, 20, 1, 30));
    fn.body.conduits.emplace(40, makeConduit(40, 10, {{.target = 30, .index = 0}}));
    fn.body.conduits.emplace(42, makeConduit(42, 11, {{.target = 31, .index = 0}}));
    fn.body.conduits.emplace(43, makeConduit(43, 30, {{.target = 31, .index = 0}}));
    fn.body.conduits.emplace(44, makeConduit(44, 11, {{.target = 30, .index = 1}, {.target = 31, .index = 0}}));

    fluir::pt::ParseTree tree;
    tree.header = fluir::pt::Header{.version = {0, 1, 3}};
    tree.declarations.emplace(fn.id, fluir::pt::Declaration{fn});
    return tree;
  }

  // makeTree plus top-level comment 5.
  fluir::pt::ParseTree makeTreeWithComment() {
    fluir::pt::ParseTree tree = makeTree();
    tree.declarations.emplace(
      5,
      fluir::pt::Declaration{fluir::pt::Comment{
        .id = 5, .location = FlowGraphLocation{.x = 3, .y = 4, .z = 1, .width = 25, .height = 25}, .text = "hi"}});
    return tree;
  }

  const fluir::pt::Block& bodyOf(const fluir::pt::ParseTree& tree) {
    return std::get<fluir::pt::FunctionDecl>(tree.declarations.at(1)).body;
  }

}  // namespace

TEST(MoveTransaction, RoundTripRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  MoveTransaction uut{FullID{1, 10}, 7, 9};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(locationAt(tree, FullID{1, 10})->x, 7);
  EXPECT_EQ(locationAt(tree, FullID{1, 10})->y, 9);
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(MoveTransaction, MovesAFunction) {
  fluir::pt::ParseTree tree = makeTree();

  MoveTransaction uut{FullID{1}, 7, 9};
  ASSERT_TRUE(uut.execute(tree));

  EXPECT_EQ(locationAt(tree, FullID{1})->x, 7);
}

TEST(MoveTransaction, MovesATopLevelComment) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;

  MoveTransaction uut{FullID{5}, 7, 9};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(locationAt(tree, FullID{5})->x, 7);
  EXPECT_EQ(locationAt(tree, FullID{5})->y, 9);
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(MoveTransaction, RedoReachesTheSameStateAsTheFirstExecute) {
  fluir::pt::ParseTree tree = makeTree();

  MoveTransaction uut{FullID{1, 10}, 7, 9};
  ASSERT_TRUE(uut.execute(tree));
  const fluir::pt::ParseTree afterFirst = tree;
  ASSERT_TRUE(uut.unexecute(tree));
  ASSERT_TRUE(uut.execute(tree));

  EXPECT_EQ(tree, afterFirst);
}

TEST(MoveTransaction, MissChangesNothingAndReturnsFalse) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  MoveTransaction unknown{FullID{1, 999}, 7, 9};
  EXPECT_FALSE(unknown.execute(tree));

  // A drag that ended where it started is a no-op swap, not an edit.
  MoveTransaction inPlace{FullID{1, 10}, 1, 1};
  EXPECT_FALSE(inPlace.execute(tree));

  EXPECT_EQ(tree, before);
}

TEST(ResizeTransaction, RoundTripRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  ResizeTransaction uut{FullID{1, 10}, 9, 7};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(locationAt(tree, FullID{1, 10})->width, 9);
  EXPECT_EQ(locationAt(tree, FullID{1, 10})->height, 7);
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(ResizeTransaction, ResizeToTheCurrentSizeChangesNothing) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  ResizeTransaction uut{FullID{1, 10}, 5, 5};
  EXPECT_FALSE(uut.execute(tree));
  EXPECT_EQ(tree, before);
}

TEST(ResizeTransaction, ResizeOfAnUnknownPathFails) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  ResizeTransaction unknown{FullID{1, 999}, 9, 7};
  EXPECT_FALSE(unknown.execute(tree));
  EXPECT_EQ(tree, before);
}

TEST(DeleteTransaction, RoundTripRestoresNodeConduitsAndOperands) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  DeleteTransaction uut{FullID{1, 30}};
  ASSERT_TRUE(uut.execute(tree));

  const fluir::pt::Block& body = bodyOf(tree);
  EXPECT_EQ(body.nodes.count(30), 0u);
  EXPECT_EQ(body.conduits.count(40), 0u);               // targeted the node
  EXPECT_EQ(body.conduits.count(43), 0u);               // sourced from the node
  EXPECT_EQ(body.conduits.count(42), 1u);               // the control, untouched
  ASSERT_EQ(body.conduits.at(44).children.size(), 1u);  // only the matching target stripped
  EXPECT_EQ(std::get<fluir::pt::Unary>(body.nodes.at(31)).lhs, fluir::INVALID_ID);

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(DeleteTransaction, RedoOfANodeReachesTheSameStateAsTheFirstExecute) {
  fluir::pt::ParseTree tree = makeTree();

  DeleteTransaction uut{FullID{1, 30}};
  ASSERT_TRUE(uut.execute(tree));
  const fluir::pt::ParseTree afterFirst = tree;
  ASSERT_TRUE(uut.unexecute(tree));
  ASSERT_TRUE(uut.execute(tree));

  EXPECT_EQ(tree, afterFirst);
}

TEST(DeleteTransaction, RoundTripRestoresTheWholeFunction) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  DeleteTransaction uut{FullID{1}};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_TRUE(tree.declarations.empty());

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(DeleteTransaction, RedoOfAFunctionReachesTheSameStateAsTheFirstExecute) {
  fluir::pt::ParseTree tree = makeTree();

  DeleteTransaction uut{FullID{1}};
  ASSERT_TRUE(uut.execute(tree));
  const fluir::pt::ParseTree afterFirst = tree;
  ASSERT_TRUE(uut.unexecute(tree));
  ASSERT_TRUE(uut.execute(tree));

  EXPECT_EQ(tree, afterFirst);
}

TEST(DeleteTransaction, RoundTripAndRedoOfATopLevelComment) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;

  DeleteTransaction uut{FullID{5}};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_FALSE(tree.declarations.contains(5));
  EXPECT_TRUE(tree.declarations.contains(1));
  const fluir::pt::ParseTree afterFirst = tree;

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(tree, afterFirst);
}

TEST(DeleteTransaction, UnresolvedPathsChangeNothingAndReturnFalse) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((DeleteTransaction{FullID{1, 999}}).execute(tree));
  EXPECT_FALSE((DeleteTransaction{FullID{999}}).execute(tree));
  EXPECT_FALSE((DeleteTransaction{FullID{}}).execute(tree));
  // A conduit id must not be mistaken for a node.
  EXPECT_FALSE((DeleteTransaction{FullID{1, 40}}).execute(tree));
  EXPECT_EQ(tree, before);
}

TEST(DeleteTransaction, UnexecuteWithoutExecuteReturnsFalse) {
  fluir::pt::ParseTree tree = makeTree();

  EXPECT_FALSE((DeleteTransaction{FullID{1, 30}}).unexecute(tree));
}

TEST(SetConstantValueTransaction, ReplacesTheLiteralAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  SetConstantValueTransaction uut{FullID{1, 10}, fluir::literals_types::I32{42}};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(std::get<fluir::pt::Constant>(bodyOf(tree).nodes.at(10)).value,
            fluir::pt::Literal{fluir::literals_types::I32{42}});
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(SetConstantValueTransaction, KeepsTheConstantsType) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  // Node 10 holds an I32; an F64 is a different alternative, so it is rejected.
  SetConstantValueTransaction uut{FullID{1, 10}, fluir::literals_types::F64{4.2}};
  EXPECT_FALSE(uut.execute(tree));
  EXPECT_EQ(tree, before);
}

TEST(SetConstantValueTransaction, SameLiteralMissingNodeOrNonConstantChangeNothing) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((SetConstantValueTransaction{FullID{1, 10}, fluir::literals_types::I32{0}}).execute(tree));
  EXPECT_FALSE((SetConstantValueTransaction{FullID{1, 999}, fluir::literals_types::I32{42}}).execute(tree));
  EXPECT_FALSE((SetConstantValueTransaction{FullID{1, 30}, fluir::literals_types::I32{42}}).execute(tree));
  EXPECT_EQ(tree, before);
}
