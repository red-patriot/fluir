#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/commands/add_parameter.hpp"
#include "editor/commands/add_return.hpp"
#include "editor/commands/delete_parameter.hpp"
#include "editor/commands/delete_return.hpp"
#include "editor/core/field.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/field_text.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/parameter.hpp"
#include "editor/diagram/return.hpp"
#include "interaction.hpp"

namespace {

  namespace diagram = fluir::editor::diagram;
  namespace pt = fluir::pt;
  using fluir::editor::AddParameter;
  using fluir::editor::AddReturn;
  using fluir::editor::DeleteParameter;
  using fluir::editor::DeleteReturn;
  using fluir::editor::Field;
  using testutil::Interaction;

  pt::FunctionDecl& fnOf(pt::ParseTree& tree) { return std::get<pt::FunctionDecl>(tree.declarations.at(1)); }

}  // namespace

TEST(AddParameter, AppendsAnI32ParameterAfterTheLastAndUndoRemovesIt) {
  Interaction h("interaction/wired_function.fl");
  auto& function = h.find<diagram::Function>(1);
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  fnOf(expected).input->parameters.push_back({.id = 10, .index = 1, .name = "param2", .typeName = "I32"});

  h.stack().push(new AddParameter(function));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(Interaction::fieldOf<diagram::FieldText>(function, {Field::Kind::ParamName, 1}).toPlainText(), "param2");
  EXPECT_NO_THROW(Interaction::terminalOf(h.find<diagram::Parameter>(10), true, 0));
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_EQ(testutil::all<diagram::Parameter>(h.scene()).size(), 1U);
  h.stack().redo();
  EXPECT_EQ(h.tree(), expected);
}

TEST(AddParameter, AddsAnInputBlockToAFunctionWithoutOne) {
  Interaction h("read/single_empty_function.fl");
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  fnOf(expected).input =
    pt::FunctionInputBlock{.parameters = {{.id = 1, .index = 0, .name = "param1", .typeName = "I32"}}};

  h.stack().push(new AddParameter(h.find<diagram::Function>(1)));

  EXPECT_EQ(h.tree(), expected);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
}

TEST(AddReturn, AddsAnI32ReturnAndUndoRemovesIt) {
  Interaction h("read/single_empty_function.fl");
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  fnOf(expected).output = pt::FunctionOutputBlock{.ret = pt::FunctionReturn{.id = 1, .typeName = "I32"}};

  h.stack().push(new AddReturn(h.find<diagram::Function>(1)));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_NO_THROW(Interaction::terminalOf(h.find<diagram::Return>(1), false, 0));
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_TRUE(testutil::all<diagram::Return>(h.scene()).empty());
  h.stack().redo();
  EXPECT_EQ(h.tree(), expected);
}

TEST(DeleteParameter, DropsItsConduitsAndTheLastLeavesAnEmptyInputBlock) {
  Interaction h("interaction/wired_function.fl");
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  fnOf(expected).input->parameters.clear();
  fnOf(expected).body.conduits.erase(6);

  h.stack().push(new DeleteParameter(h.find<diagram::Function>(1), 2));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_TRUE(testutil::all<diagram::Parameter>(h.scene()).empty());
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_TRUE(testutil::routed(h.find<diagram::Conduit>(6)));
  h.stack().redo();
  EXPECT_EQ(h.tree(), expected);
}

TEST(DeleteReturn, DropsTheConduitsLandingOnIt) {
  Interaction h("interaction/wired_function.fl");
  const pt::ParseTree before = h.tree();
  pt::ParseTree expected = before;
  fnOf(expected).output->ret.reset();
  fnOf(expected).body.conduits.erase(7);

  h.stack().push(new DeleteReturn(h.find<diagram::Function>(1)));

  EXPECT_EQ(h.tree(), expected);
  h.stack().undo();
  EXPECT_EQ(h.tree(), before);
  EXPECT_TRUE(testutil::routed(h.find<diagram::Conduit>(7)));
}
