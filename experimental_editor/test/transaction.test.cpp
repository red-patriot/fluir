#include "editor/transaction/transaction.hpp"

#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/transaction/edit_call_argument.hpp"
#include "editor/transaction/edit_call_node.hpp"
#include "editor/transaction/edit_comment.hpp"
#include "editor/transaction/edit_operator.hpp"
#include "editor/transaction/move.hpp"
#include "editor/transaction/rename.hpp"
#include "editor/transaction/resize.hpp"
#include "editor/transaction/set_constant_value.hpp"
#include "editor/transaction/update_func_param.hpp"

// The house round trip: apply, assert, reverse, assert the tree is back.

namespace {

  using fluir::FlowGraphLocation;
  using fluir::FullID;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::DeleteTransaction;
  using fluir::editor::EditCallArgumentTransaction;
  using fluir::editor::EditCallNodeTransaction;
  using fluir::editor::EditCommentTransaction;
  using fluir::editor::EditOperatorTransaction;
  using fluir::editor::functionAt;
  using fluir::editor::locationAt;
  using fluir::editor::MoveTransaction;
  using fluir::editor::nodeAt;
  using fluir::editor::RenameTransaction;
  using fluir::editor::ResizeTransaction;
  using fluir::editor::SetConstantValueTransaction;
  using fluir::editor::UpdateFuncParamTransaction;

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

  fluir::pt::Call makeCall(ID id, int x, int y) {
    return fluir::pt::Call{.id = id,
                           .location = FlowGraphLocation{.x = x, .y = y, .z = 1, .width = 5, .height = 5},
                           .target = "g",
                           ._return = fluir::pt::Call::Return{},
                           .arguments = {{.name = "a", .index = 0}, {.name = "b", .index = 1}}};
  }

  fluir::pt::Conduit makeConduit(ID id, ID input, std::vector<fluir::pt::Conduit::Output> children) {
    return fluir::pt::Conduit{.id = id, .input = input, .index = 0, .children = std::move(children)};
  }

  // Two constants feed binary 30, whose result feeds unary 31. Conduit 44 fans
  // out to both 30 and 31, so deleting 30 only strips one of its targets. Call 32 targets "g".
  // Function 1 takes parameters x (index 0) and y (index 1).
  fluir::pt::ParseTree makeTree() {
    fluir::pt::FunctionDecl fn;
    fn.id = 1;
    fn.location = FlowGraphLocation{.x = 0, .y = 0, .z = 0, .width = 100, .height = 100};
    fn.name = "f";
    fn.input =
      fluir::pt::FunctionDecl::InputBlock{.parameters = {{.id = 2, .index = 0, .name = "x", .typeName = "i32"},
                                                         {.id = 3, .index = 1, .name = "y", .typeName = "i32"}}};
    fn.body.nodes.emplace(10, makeConstant(10, 1, 1));
    fn.body.nodes.emplace(11, makeConstant(11, 1, 10));
    fn.body.nodes.emplace(30, makeBinary(30, 10, 1, 10, 11));
    fn.body.nodes.emplace(31, makeUnary(31, 20, 1, 30));
    fn.body.nodes.emplace(32, makeCall(32, 30, 1));
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

TEST(RenameTransaction, RenamesTheFunctionAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  RenameTransaction uut{FullID{1}, "add_two"};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(functionAt(tree, FullID{1})->name, "add_two");
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(RenameTransaction, RedoReachesTheSameStateAsTheFirstExecute) {
  fluir::pt::ParseTree tree = makeTree();

  RenameTransaction uut{FullID{1}, "add_two"};
  ASSERT_TRUE(uut.execute(tree));
  const fluir::pt::ParseTree afterFirst = tree;
  ASSERT_TRUE(uut.unexecute(tree));
  ASSERT_TRUE(uut.execute(tree));

  EXPECT_EQ(tree, afterFirst);
}

TEST(RenameTransaction, SameNameUnresolvedPathsOrInvalidNamesChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((RenameTransaction{FullID{1}, "f"}).execute(tree));
  EXPECT_FALSE((RenameTransaction{FullID{999}, "h"}).execute(tree));
  EXPECT_FALSE((RenameTransaction{FullID{1, 30}, "h"}).execute(tree));
  EXPECT_FALSE((RenameTransaction{FullID{5}, "h"}).execute(tree));
  EXPECT_FALSE((RenameTransaction{FullID{1}, "1bad"}).execute(tree));
  EXPECT_FALSE((RenameTransaction{FullID{1}, ""}).execute(tree));
  EXPECT_EQ(tree, before);
}

TEST(EditCallNodeTransaction, RetargetsTheCallAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  EditCallNodeTransaction uut{FullID{1, 32}, "h"};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(std::get<fluir::pt::Call>(*nodeAt(tree, FullID{1, 32})).target, "h");
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(EditCallNodeTransaction, LeavesArgumentsAndReturnUntouched) {
  fluir::pt::ParseTree tree = makeTree();
  const auto original = std::get<fluir::pt::Call>(*nodeAt(tree, FullID{1, 32}));

  ASSERT_TRUE((EditCallNodeTransaction{FullID{1, 32}, "h"}).execute(tree));

  const auto& call = std::get<fluir::pt::Call>(*nodeAt(tree, FullID{1, 32}));
  EXPECT_EQ(call.arguments, original.arguments);
  EXPECT_EQ(call._return, original._return);
}

TEST(EditCallNodeTransaction, SameTargetMissingNodeNonCallOrInvalidTargetChangeNothing) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((EditCallNodeTransaction{FullID{1, 32}, "g"}).execute(tree));
  EXPECT_FALSE((EditCallNodeTransaction{FullID{1, 999}, "h"}).execute(tree));
  EXPECT_FALSE((EditCallNodeTransaction{FullID{1, 30}, "h"}).execute(tree));
  EXPECT_FALSE((EditCallNodeTransaction{FullID{1, 32}, "a-b"}).execute(tree));
  EXPECT_FALSE((EditCallNodeTransaction{FullID{1, 32}, ""}).execute(tree));
  EXPECT_EQ(tree, before);
}

TEST(UpdateFuncParamTransaction, RenamesTheParameterAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  UpdateFuncParamTransaction uut{FullID{1}, 1, "z"};
  ASSERT_TRUE(uut.execute(tree));
  const auto& params = functionAt(tree, FullID{1})->input->parameters;
  EXPECT_EQ(params[1].name, "z");
  EXPECT_EQ(params[1].typeName, "i32");
  EXPECT_EQ(params[0], functionAt(before, FullID{1})->input->parameters[0]);
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(UpdateFuncParamTransaction, SameNameUnresolvedTargetsOrInvalidNamesChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((UpdateFuncParamTransaction{FullID{1}, 1, "y"}).execute(tree));
  EXPECT_FALSE((UpdateFuncParamTransaction{FullID{999}, 1, "z"}).execute(tree));
  EXPECT_FALSE((UpdateFuncParamTransaction{FullID{1, 30}, 1, "z"}).execute(tree));
  EXPECT_FALSE((UpdateFuncParamTransaction{FullID{5}, 1, "z"}).execute(tree));
  EXPECT_FALSE((UpdateFuncParamTransaction{FullID{1}, 7, "z"}).execute(tree));
  EXPECT_FALSE((UpdateFuncParamTransaction{FullID{1}, 1, "1x"}).execute(tree));
  EXPECT_FALSE((UpdateFuncParamTransaction{FullID{1}, 1, ""}).execute(tree));
  EXPECT_EQ(tree, before);
}

TEST(EditCallArgumentTransaction, RenamesTheArgumentAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;
  const auto original = std::get<fluir::pt::Call>(*nodeAt(tree, FullID{1, 32}));

  EditCallArgumentTransaction uut{FullID{1, 32}, 1, "c"};
  ASSERT_TRUE(uut.execute(tree));
  const auto& call = std::get<fluir::pt::Call>(*nodeAt(tree, FullID{1, 32}));
  EXPECT_EQ(call.arguments[1].name, "c");
  EXPECT_EQ(call.arguments[0], original.arguments[0]);
  EXPECT_EQ(call.target, original.target);
  EXPECT_EQ(call._return, original._return);
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(EditCallArgumentTransaction, SameNameMissingNodeNonCallMissingIndexOrInvalidNameChangeNothing) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((EditCallArgumentTransaction{FullID{1, 32}, 1, "b"}).execute(tree));
  EXPECT_FALSE((EditCallArgumentTransaction{FullID{1, 999}, 1, "c"}).execute(tree));
  EXPECT_FALSE((EditCallArgumentTransaction{FullID{1, 30}, 1, "c"}).execute(tree));
  EXPECT_FALSE((EditCallArgumentTransaction{FullID{1, 32}, 7, "c"}).execute(tree));
  EXPECT_FALSE((EditCallArgumentTransaction{FullID{1, 32}, 1, "1x"}).execute(tree));
  EXPECT_FALSE((EditCallArgumentTransaction{FullID{1, 32}, 1, ""}).execute(tree));
  EXPECT_EQ(tree, before);
}

namespace {

  const std::string& commentText(const fluir::pt::ParseTree& tree, const FullID& path) {
    if (path.size() == 1) {
      return std::get<fluir::pt::Comment>(tree.declarations.at(path[0])).text;
    }
    return std::get<fluir::pt::Comment>(*nodeAt(tree, path)).text;
  }

}  // namespace

TEST(EditCommentTransaction, EditsATopLevelCommentAndUndoRestoresIt) {
  for (const std::string text : {"", "hello, world! (x + y) -- done."}) {
    fluir::pt::ParseTree tree = makeTreeWithComment();
    const fluir::pt::ParseTree before = tree;

    EditCommentTransaction uut{FullID{5}, text};
    ASSERT_TRUE(uut.execute(tree)) << text;
    EXPECT_EQ(commentText(tree, FullID{5}), text);
    const fluir::pt::ParseTree afterFirst = tree;
    ASSERT_TRUE(uut.unexecute(tree)) << text;
    EXPECT_EQ(tree, before);
    ASSERT_TRUE(uut.execute(tree)) << text;
    EXPECT_EQ(tree, afterFirst);
  }
}

TEST(EditCommentTransaction, EditsAnInBodyCommentAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  std::get<fluir::pt::FunctionDecl>(tree.declarations.at(1))
    .body.nodes.emplace(
      50,
      fluir::pt::Comment{
        .id = 50, .location = FlowGraphLocation{.x = 40, .y = 40, .z = 1, .width = 10, .height = 10}, .text = "in"});
  const fluir::pt::ParseTree before = tree;

  EditCommentTransaction uut{FullID{1, 50}, "inside body"};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(commentText(tree, FullID{1, 50}), "inside body");
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(EditCommentTransaction, SameTextMissingPathFunctionOrNonCommentChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((EditCommentTransaction{FullID{5}, "hi"}).execute(tree));
  EXPECT_FALSE((EditCommentTransaction{FullID{999}, "x"}).execute(tree));
  EXPECT_FALSE((EditCommentTransaction{FullID{1}, "x"}).execute(tree));
  EXPECT_FALSE((EditCommentTransaction{FullID{1, 30}, "x"}).execute(tree));
  EXPECT_FALSE((EditCommentTransaction{FullID{1, 999}, "x"}).execute(tree));
  EXPECT_EQ(tree, before);
}

TEST(EditOperatorTransaction, ChangesABinaryOperatorAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  EditOperatorTransaction uut{FullID{1, 30}, Operator::STAR};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(std::get<fluir::pt::Binary>(*nodeAt(tree, FullID{1, 30})).op, Operator::STAR);
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(EditOperatorTransaction, ChangesAUnaryOperatorAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  EditOperatorTransaction uut{FullID{1, 31}, Operator::BANG};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(std::get<fluir::pt::Unary>(*nodeAt(tree, FullID{1, 31})).op, Operator::BANG);
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(EditOperatorTransaction, LeavesOperandsUntouched) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::Binary before = std::get<fluir::pt::Binary>(*nodeAt(tree, FullID{1, 30}));

  ASSERT_TRUE((EditOperatorTransaction{FullID{1, 30}, Operator::SLASH}).execute(tree));
  const auto& after = std::get<fluir::pt::Binary>(*nodeAt(tree, FullID{1, 30}));
  EXPECT_EQ(after.lhs, before.lhs);
  EXPECT_EQ(after.rhs, before.rhs);
  EXPECT_EQ(after.location, before.location);
}

TEST(EditOperatorTransaction, SameOpMissingNodeNonOperatorOrUnknownChangeNothing) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((EditOperatorTransaction{FullID{1, 30}, Operator::PLUS}).execute(tree));
  EXPECT_FALSE((EditOperatorTransaction{FullID{1, 999}, Operator::STAR}).execute(tree));
  EXPECT_FALSE((EditOperatorTransaction{FullID{1, 32}, Operator::STAR}).execute(tree));
  EXPECT_FALSE((EditOperatorTransaction{FullID{1}, Operator::STAR}).execute(tree));
  EXPECT_FALSE((EditOperatorTransaction{FullID{1, 30}, Operator::UNKNOWN}).execute(tree));
  EXPECT_EQ(tree, before);
}
