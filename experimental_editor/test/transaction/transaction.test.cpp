#include "editor/transaction/transaction.hpp"

#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/transaction/add_comment.hpp"
#include "editor/transaction/add_conduit.hpp"
#include "editor/transaction/add_decl.hpp"
#include "editor/transaction/add_node.hpp"
#include "editor/transaction/add_parameter.hpp"
#include "editor/transaction/add_return.hpp"
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
  using fluir::editor::AddComment;
  using fluir::editor::AddConduit;
  using fluir::editor::AddDecl;
  using fluir::editor::AddNode;
  using fluir::editor::AddParameter;
  using fluir::editor::AddReturn;
  using fluir::editor::blockOf;
  using fluir::editor::ConstantOption;
  using fluir::editor::DeleteTransaction;
  using fluir::editor::EditCallArgumentTransaction;
  using fluir::editor::EditCallNodeTransaction;
  using fluir::editor::EditCommentTransaction;
  using fluir::editor::EditOperatorTransaction;
  using fluir::editor::functionAt;
  using fluir::editor::locationAt;
  using fluir::editor::MoveTransaction;
  using fluir::editor::nodeAt;
  using fluir::editor::OperatorOption;
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

TEST(DeleteTransaction, DeletingAParameterDropsItsConduitsAndUndoRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTree();
  std::get<fluir::pt::FunctionDecl>(tree.declarations.at(1))
    .body.conduits.emplace(45, makeConduit(45, 2, {{.target = 30, .index = 0}}));
  const fluir::pt::ParseTree before = tree;

  DeleteTransaction uut{FullID{1, 2}};
  ASSERT_TRUE(uut.execute(tree));
  const auto& params = functionAt(tree, FullID{1})->input->parameters;
  ASSERT_EQ(params.size(), 1u);
  EXPECT_EQ(params[0], (fluir::pt::FunctionDecl::Parameter{.id = 3, .index = 1, .name = "y", .typeName = "i32"}));
  EXPECT_FALSE(bodyOf(tree).conduits.contains(45));
  EXPECT_EQ(bodyOf(tree).conduits.size(), 4u);
  const fluir::pt::ParseTree afterFirst = tree;

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(tree, afterFirst);
}

TEST(DeleteTransaction, DeletingTheReturnStripsConduitsLandingOnItAndUndoRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTree();
  auto& fn = std::get<fluir::pt::FunctionDecl>(tree.declarations.at(1));
  fn.output = fluir::pt::FunctionDecl::OutputBlock{.ret = fluir::pt::FunctionDecl::Return{.id = 4, .typeName = "I32"}};
  fn.body.conduits.emplace(46, makeConduit(46, 31, {{.target = 4, .index = 0}}));
  fn.body.conduits.emplace(47, makeConduit(47, 30, {{.target = 4, .index = 0}, {.target = 32, .index = 0}}));
  const fluir::pt::ParseTree before = tree;

  DeleteTransaction uut{FullID{1, 4}};
  ASSERT_TRUE(uut.execute(tree));
  const auto* after = functionAt(tree, FullID{1});
  ASSERT_TRUE(after->output.has_value());
  EXPECT_FALSE(after->output->ret.has_value());
  EXPECT_FALSE(after->body.conduits.contains(46));
  ASSERT_TRUE(after->body.conduits.contains(47));
  EXPECT_EQ(after->body.conduits.at(47).children,
            (std::vector<fluir::pt::Conduit::Output>{{.target = 32, .index = 0}}));
  const fluir::pt::ParseTree afterFirst = tree;

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(tree, afterFirst);
}

TEST(DeleteTransaction, DeletingTheOnlyParameterLeavesAnEmptyInputBlock) {
  fluir::pt::ParseTree tree = makeTree();
  functionAt(tree, FullID{1})->input->parameters.pop_back();
  const fluir::pt::ParseTree before = tree;

  DeleteTransaction uut{FullID{1, 2}};
  ASSERT_TRUE(uut.execute(tree));
  const auto* fn = functionAt(tree, FullID{1});
  ASSERT_TRUE(fn->input.has_value());
  EXPECT_TRUE(fn->input->parameters.empty());

  ASSERT_TRUE(uut.unexecute(tree));
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

TEST(SetConstantValueTransaction, TogglesABool) {
  fluir::pt::ParseTree tree = makeTree();
  std::get<fluir::pt::Constant>(std::get<fluir::pt::FunctionDecl>(tree.declarations.at(1)).body.nodes.at(10)).value =
    fluir::literals_types::BOOL{false};
  const fluir::pt::ParseTree before = tree;

  SetConstantValueTransaction uut{FullID{1, 10}, fluir::literals_types::BOOL{true}};
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(std::get<fluir::pt::Constant>(bodyOf(tree).nodes.at(10)).value,
            fluir::pt::Literal{fluir::literals_types::BOOL{true}});
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

  const auto uut = UpdateFuncParamTransaction::rename(FullID{1}, 1, "z");
  ASSERT_TRUE(uut->execute(tree));
  const auto& params = functionAt(tree, FullID{1})->input->parameters;
  EXPECT_EQ(params[1].name, "z");
  EXPECT_EQ(params[1].typeName, "i32");
  EXPECT_EQ(params[0], functionAt(before, FullID{1})->input->parameters[0]);
  ASSERT_TRUE(uut->unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(UpdateFuncParamTransaction, SameNameUnresolvedTargetsOrInvalidNamesChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE(UpdateFuncParamTransaction::rename(FullID{1}, 1, "y")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::rename(FullID{999}, 1, "z")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::rename(FullID{1, 30}, 1, "z")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::rename(FullID{5}, 1, "z")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::rename(FullID{1}, 7, "z")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::rename(FullID{1}, 1, "1x")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::rename(FullID{1}, 1, "")->execute(tree));
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

TEST(UpdateFuncParamTransaction, SetsAParamTypeAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  const auto uut = UpdateFuncParamTransaction::setType(FullID{1}, 3, "F64");
  ASSERT_TRUE(uut->execute(tree));
  const auto& params = functionAt(tree, FullID{1})->input->parameters;
  EXPECT_EQ(params[1].typeName, "F64");
  EXPECT_EQ(params[1].name, "y");
  EXPECT_EQ(params[0], functionAt(before, FullID{1})->input->parameters[0]);
  ASSERT_TRUE(uut->unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(UpdateFuncParamTransaction, SetsAReturnTypeAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  functionAt(tree, FullID{1})->output =
    fluir::pt::FunctionDecl::OutputBlock{.ret = fluir::pt::FunctionDecl::Return{.id = 4, .typeName = "F64"}};
  const fluir::pt::ParseTree before = tree;

  const auto uut = UpdateFuncParamTransaction::setType(FullID{1}, 4, "BOOL");
  ASSERT_TRUE(uut->execute(tree));
  EXPECT_EQ(functionAt(tree, FullID{1})->output->ret->typeName, "BOOL");
  ASSERT_TRUE(uut->unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(UpdateFuncParamTransaction, SameTypeMissingFunctionMissingRailOrEmptyTypeChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE(UpdateFuncParamTransaction::setType(FullID{1}, 3, "i32")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::setType(FullID{999}, 3, "F64")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::setType(FullID{5}, 3, "F64")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::setType(FullID{1}, 99, "F64")->execute(tree));
  EXPECT_FALSE(UpdateFuncParamTransaction::setType(FullID{1}, 3, "")->execute(tree));
  EXPECT_EQ(tree, before);
}

namespace {

  constexpr FlowGraphLocation kNewLocation{.x = 7, .y = 8, .z = 1, .width = 40, .height = 30};

}  // namespace

TEST(AddDecl, AddsAnEmptyFunctionAndUndoRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  AddDecl uut{FullID{}, 50, kNewLocation};
  ASSERT_TRUE(uut.execute(tree));
  const auto* fn = functionAt(tree, FullID{50});
  ASSERT_NE(fn, nullptr);
  EXPECT_EQ(fn->id, 50u);
  EXPECT_EQ(fn->location, kNewLocation);
  EXPECT_EQ(fn->name, "new_function");
  EXPECT_EQ(fn->body, fluir::pt::Block{});
  const fluir::pt::ParseTree afterFirst = tree;

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(tree, afterFirst);
}

TEST(AddDecl, NonTopLevelParentTakenOrInvalidIdChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((AddDecl{FullID{1}, 50, kNewLocation}).execute(tree));
  EXPECT_FALSE((AddDecl{FullID{}, 1, kNewLocation}).execute(tree));
  EXPECT_FALSE((AddDecl{FullID{}, 5, kNewLocation}).execute(tree));
  EXPECT_FALSE((AddDecl{FullID{}, fluir::INVALID_ID, kNewLocation}).execute(tree));
  EXPECT_FALSE((AddDecl{FullID{}, 50, kNewLocation}).unexecute(tree)) << "never added";
  EXPECT_EQ(tree, before);
}

TEST(AddComment, AddsATopLevelCommentAndUndoRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  AddComment uut{FullID{}, 50, kNewLocation};
  ASSERT_TRUE(uut.execute(tree));
  const auto* decl = fluir::editor::declarationAt(tree, FullID{50});
  ASSERT_NE(decl, nullptr);
  EXPECT_EQ(*decl, (fluir::pt::Declaration{fluir::pt::Comment{.id = 50, .location = kNewLocation, .text = ""}}));
  const fluir::pt::ParseTree afterFirst = tree;

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(tree, afterFirst);
}

TEST(AddComment, AddsAnInBodyCommentAndUndoRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  AddComment uut{FullID{1}, 50, kNewLocation};
  ASSERT_TRUE(uut.execute(tree));
  const auto* node = nodeAt(tree, FullID{1, 50});
  ASSERT_NE(node, nullptr);
  EXPECT_EQ(*node, (fluir::pt::Node{fluir::pt::Comment{.id = 50, .location = kNewLocation, .text = ""}}));
  EXPECT_FALSE(tree.declarations.contains(50));
  const fluir::pt::ParseTree afterFirst = tree;

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(tree, afterFirst);
}

TEST(AddComment, TakenOrInvalidIdOrUnresolvedParentChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((AddComment{FullID{}, 5, kNewLocation}).execute(tree));
  EXPECT_FALSE((AddComment{FullID{1}, 30, kNewLocation}).execute(tree));
  EXPECT_FALSE((AddComment{FullID{}, fluir::INVALID_ID, kNewLocation}).execute(tree));
  EXPECT_FALSE((AddComment{FullID{999}, 50, kNewLocation}).execute(tree));
  EXPECT_FALSE((AddComment{FullID{5}, 50, kNewLocation}).execute(tree)) << "a comment has no body";
  EXPECT_FALSE((AddComment{FullID{1}, 50, kNewLocation}).unexecute(tree)) << "never added";
  EXPECT_EQ(tree, before);
}

namespace {

  constexpr FlowGraphLocation kNodeLocation{.x = 7, .y = 8, .z = 1, .width = 8, .height = 5};

  // Executes, checks node 50 equals `want`, then reverses and redoes.
  void expectAddNodeRoundTrip(AddNode& uut, const fluir::pt::Node& want) {
    fluir::pt::ParseTree tree = makeTree();
    const fluir::pt::ParseTree before = tree;

    ASSERT_TRUE(uut.execute(tree));
    const auto* node = nodeAt(tree, FullID{1, 50});
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(*node, want);
    const fluir::pt::ParseTree afterFirst = tree;

    ASSERT_TRUE(uut.unexecute(tree));
    EXPECT_EQ(tree, before);
    ASSERT_TRUE(uut.execute(tree));
    EXPECT_EQ(tree, afterFirst);
  }

}  // namespace

TEST(AddNode, AddsABinaryOperatorAndUndoRestoresTheTree) {
  AddNode uut{FullID{1}, 50, kNodeLocation, OperatorOption{Operator::STAR, OperatorOption::BINARY}};

  expectAddNodeRoundTrip(uut, fluir::pt::Binary{.id = 50, .location = kNodeLocation, .op = Operator::STAR});
}

TEST(AddNode, AddsAUnaryOperatorAndUndoRestoresTheTree) {
  AddNode uut{FullID{1}, 50, kNodeLocation, OperatorOption{Operator::BANG, OperatorOption::UNARY}};

  expectAddNodeRoundTrip(uut, fluir::pt::Unary{.id = 50, .location = kNodeLocation, .op = Operator::BANG});
}

TEST(AddNode, AddsAConstantAndUndoRestoresTheTree) {
  AddNode uut{FullID{1}, 50, kNodeLocation, ConstantOption{fluir::literals_types::U16{0}}};

  expectAddNodeRoundTrip(
    uut, fluir::pt::Constant{.id = 50, .location = kNodeLocation, .value = fluir::literals_types::U16{0}});
}

TEST(AddNode, TakenOrInvalidIdOrUnresolvedParentOrUnknownOperatorChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;
  const OperatorOption plus{Operator::PLUS, OperatorOption::BINARY};

  EXPECT_FALSE((AddNode{FullID{1}, 30, kNodeLocation, plus}).execute(tree)) << "taken";
  EXPECT_FALSE((AddNode{FullID{1}, fluir::INVALID_ID, kNodeLocation, plus}).execute(tree)) << "invalid";
  EXPECT_FALSE((AddNode{FullID{}, 50, kNodeLocation, plus}).execute(tree)) << "top level";
  EXPECT_FALSE((AddNode{FullID{999}, 50, kNodeLocation, plus}).execute(tree)) << "unresolved";
  EXPECT_FALSE((AddNode{FullID{5}, 50, kNodeLocation, plus}).execute(tree)) << "a comment has no body";
  EXPECT_FALSE(
    (AddNode{FullID{1}, 50, kNodeLocation, OperatorOption{Operator::UNKNOWN, OperatorOption::UNARY}}).execute(tree))
    << "unknown operator";
  EXPECT_FALSE((AddNode{FullID{1}, 50, kNodeLocation, plus}).unexecute(tree)) << "never added";
  EXPECT_EQ(tree, before);
}

namespace {

  // Executes on `tree`, checks the new conduit equals `want`, then reverses and redoes.
  void expectAddConduitRoundTrip(AddConduit& uut, fluir::pt::ParseTree tree, const fluir::pt::Conduit& want) {
    const fluir::pt::ParseTree before = tree;

    ASSERT_TRUE(uut.execute(tree));
    const fluir::pt::Block* body = blockOf(tree, FullID{1});
    ASSERT_NE(body, nullptr);
    ASSERT_TRUE(body->conduits.contains(want.id));
    EXPECT_EQ(body->conduits.at(want.id), want);
    const fluir::pt::ParseTree afterFirst = tree;

    ASSERT_TRUE(uut.unexecute(tree));
    EXPECT_EQ(tree, before);
    ASSERT_TRUE(uut.execute(tree));
    EXPECT_EQ(tree, afterFirst);
  }

}  // namespace

TEST(AddConduit, ConnectsAnOutputToAnInputAndUndoRestoresTheTree) {
  AddConduit uut{FullID{1}, 50, {.node = 10, .index = 0}, {.node = 32, .index = 0}};

  expectAddConduitRoundTrip(uut, makeTree(), makeConduit(50, 10, {{.target = 32, .index = 0}}));
}

TEST(AddConduit, ConnectsFunctionRails) {
  fluir::pt::ParseTree tree = makeTree();
  functionAt(tree, FullID{1})->output =
    fluir::pt::FunctionDecl::OutputBlock{.ret = fluir::pt::FunctionDecl::Return{.id = 4, .typeName = "i32"}};
  AddConduit fromParam{FullID{1}, 50, {.node = 3, .index = 0}, {.node = 32, .index = 1}};
  AddConduit toReturn{FullID{1}, 50, {.node = 31, .index = 0}, {.node = 4, .index = 0}};

  expectAddConduitRoundTrip(fromParam, tree, makeConduit(50, 3, {{.target = 32, .index = 1}}));
  expectAddConduitRoundTrip(toReturn, tree, makeConduit(50, 31, {{.target = 4, .index = 0}}));
}

// Conduit 40 alone feeds binary 30's input 0.
TEST(AddConduit, ReplacingAFedInputDropsItsOldConduitAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;
  AddConduit uut{FullID{1}, 50, {.node = 32, .index = 0}, {.node = 30, .index = 0}};

  ASSERT_TRUE(uut.execute(tree));
  const fluir::pt::Block& body = *blockOf(tree, FullID{1});
  EXPECT_FALSE(body.conduits.contains(40));
  EXPECT_EQ(body.conduits.at(50), makeConduit(50, 32, {{.target = 30, .index = 0}}));

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

// Conduit 44 carries constant 11 to binary 30 input 1 and unary 31.
TEST(AddConduit, ReplacingOneBranchKeepsTheRestAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;
  AddConduit uut{FullID{1}, 50, {.node = 10, .index = 0}, {.node = 30, .index = 1}};

  ASSERT_TRUE(uut.execute(tree));
  const fluir::pt::Block& body = *blockOf(tree, FullID{1});
  EXPECT_EQ(body.conduits.at(44), makeConduit(44, 11, {{.target = 31, .index = 0}}));
  EXPECT_EQ(body.conduits.at(50), makeConduit(50, 10, {{.target = 30, .index = 1}}));

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(AddConduit, InvalidOrTakenIdBadParentSameNodeOrDuplicateChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  const fluir::pt::ParseTree before = tree;
  const AddConduit::Endpoint from{.node = 10, .index = 0};
  const AddConduit::Endpoint to{.node = 32, .index = 0};

  EXPECT_FALSE((AddConduit{FullID{1}, fluir::INVALID_ID, from, to}).execute(tree)) << "invalid";
  EXPECT_FALSE((AddConduit{FullID{1}, 30, from, to}).execute(tree)) << "taken by a node";
  EXPECT_FALSE((AddConduit{FullID{1}, 40, from, to}).execute(tree)) << "taken by a conduit";
  EXPECT_FALSE((AddConduit{FullID{}, 50, from, to}).execute(tree)) << "top level";
  EXPECT_FALSE((AddConduit{FullID{999}, 50, from, to}).execute(tree)) << "unresolved";
  EXPECT_FALSE((AddConduit{FullID{5}, 50, from, to}).execute(tree)) << "a comment has no body";
  EXPECT_FALSE((AddConduit{FullID{1}, 50, {.node = 30, .index = 0}, {.node = 30, .index = 1}}).execute(tree))
    << "same node";
  EXPECT_FALSE((AddConduit{FullID{1}, 50, from, {.node = 30, .index = 0}}).execute(tree)) << "duplicate";
  EXPECT_FALSE((AddConduit{FullID{1}, 50, from, to}).unexecute(tree)) << "never added";
  EXPECT_EQ(tree, before);
}

namespace {

  // makeTree plus empty function 6.
  fluir::pt::ParseTree makeTreeWithEmptyFunction() {
    fluir::pt::ParseTree tree = makeTreeWithComment();
    tree.declarations.emplace(6,
                              fluir::pt::Declaration{fluir::pt::FunctionDecl{
                                6,
                                FlowGraphLocation{.x = 200, .y = 0, .z = 0, .width = 50, .height = 50},
                                "g",
                                {},
                                std::nullopt,
                                std::nullopt}});
    return tree;
  }

}  // namespace

TEST(AddParameter, AddsAnI32ParameterToAnEmptyFunctionAndUndoRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTreeWithEmptyFunction();
  const fluir::pt::ParseTree before = tree;

  AddParameter uut{FullID{6}, 1};
  ASSERT_TRUE(uut.execute(tree));
  const auto* fn = functionAt(tree, FullID{6});
  ASSERT_TRUE(fn->input.has_value());
  ASSERT_EQ(fn->input->parameters.size(), 1u);
  EXPECT_EQ(fn->input->parameters[0],
            (fluir::pt::FunctionDecl::Parameter{.id = 1, .index = 0, .name = "param1", .typeName = "I32"}));
  const fluir::pt::ParseTree afterFirst = tree;

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(tree, afterFirst);
}

TEST(AddParameter, AppendsAfterTheLastParameterAndUndoRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  AddParameter uut{FullID{1}, 50};
  ASSERT_TRUE(uut.execute(tree));
  const auto& params = functionAt(tree, FullID{1})->input->parameters;
  ASSERT_EQ(params.size(), 3u);
  EXPECT_EQ(params[2], (fluir::pt::FunctionDecl::Parameter{.id = 50, .index = 2, .name = "param3", .typeName = "I32"}));

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(AddParameter, SkipsANameAParameterAlreadyHas) {
  fluir::pt::ParseTree tree = makeTreeWithEmptyFunction();
  ASSERT_TRUE((AddParameter{FullID{6}, 1}).execute(tree));
  ASSERT_TRUE((AddParameter{FullID{6}, 2}).execute(tree));
  ASSERT_TRUE((DeleteTransaction{FullID{6, 1}}).execute(tree));

  ASSERT_TRUE((AddParameter{FullID{6}, 3}).execute(tree));

  const auto& params = functionAt(tree, FullID{6})->input->parameters;
  ASSERT_EQ(params.size(), 2u);
  EXPECT_EQ(params[0].name, "param2");
  EXPECT_EQ(params[1].name, "param3");
}

TEST(AddParameter, MissingFunctionInvalidOrTakenIdChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  functionAt(tree, FullID{1})->output =
    fluir::pt::FunctionDecl::OutputBlock{.ret = fluir::pt::FunctionDecl::Return{.id = 4, .typeName = "F64"}};
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((AddParameter{FullID{999}, 50}).execute(tree));
  EXPECT_FALSE((AddParameter{FullID{5}, 50}).execute(tree));
  EXPECT_FALSE((AddParameter{FullID{1}, fluir::INVALID_ID}).execute(tree));
  EXPECT_FALSE((AddParameter{FullID{1}, 3}).execute(tree));
  EXPECT_FALSE((AddParameter{FullID{1}, 4}).execute(tree));
  EXPECT_FALSE((AddParameter{FullID{1}, 50}).unexecute(tree)) << "never added";
  EXPECT_EQ(tree, before);
}

TEST(AddReturn, AddsAnI32ReturnAndUndoRestoresTheTree) {
  fluir::pt::ParseTree tree = makeTree();
  const fluir::pt::ParseTree before = tree;

  AddReturn uut{FullID{1}, 50};
  ASSERT_TRUE(uut.execute(tree));
  const auto* fn = functionAt(tree, FullID{1});
  ASSERT_TRUE(fn->output.has_value());
  EXPECT_EQ(fn->output->ret, (fluir::pt::FunctionDecl::Return{.id = 50, .typeName = "I32"}));
  const fluir::pt::ParseTree afterFirst = tree;

  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
  ASSERT_TRUE(uut.execute(tree));
  EXPECT_EQ(tree, afterFirst);
}

TEST(AddReturn, FillsAnEmptyOutputBlockAndUndoRestoresIt) {
  fluir::pt::ParseTree tree = makeTree();
  functionAt(tree, FullID{1})->output = fluir::pt::FunctionDecl::OutputBlock{};
  const fluir::pt::ParseTree before = tree;

  AddReturn uut{FullID{1}, 50};
  ASSERT_TRUE(uut.execute(tree));
  ASSERT_TRUE(uut.unexecute(tree));
  EXPECT_EQ(tree, before);
}

TEST(AddReturn, ExistingReturnMissingFunctionOrInvalidIdChangeNothing) {
  fluir::pt::ParseTree tree = makeTreeWithComment();
  functionAt(tree, FullID{1})->output =
    fluir::pt::FunctionDecl::OutputBlock{.ret = fluir::pt::FunctionDecl::Return{.id = 4, .typeName = "F64"}};
  const fluir::pt::ParseTree before = tree;

  EXPECT_FALSE((AddReturn{FullID{1}, 50}).execute(tree));
  EXPECT_FALSE((AddReturn{FullID{999}, 50}).execute(tree));
  EXPECT_FALSE((AddReturn{FullID{5}, 50}).execute(tree));
  EXPECT_FALSE((AddReturn{FullID{1}, 50}).unexecute(tree)) << "the return is not 50";
  EXPECT_EQ(tree, before);

  fluir::pt::ParseTree bare = makeTree();
  const fluir::pt::ParseTree bareBefore = bare;
  EXPECT_FALSE((AddReturn{FullID{1}, fluir::INVALID_ID}).execute(bare));
  EXPECT_FALSE((AddReturn{FullID{1}, 3}).execute(bare)) << "a parameter already has id 3";
  EXPECT_EQ(bare, bareBefore);
}
