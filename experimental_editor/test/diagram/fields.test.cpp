#include <filesystem>
#include <string>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/operator.hpp"
#include "editor/core/field.hpp"
#include "editor/diagram/binary.hpp"
#include "editor/diagram/call.hpp"
#include "editor/diagram/comment.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/field_text.hpp"
#include "editor/diagram/field_view.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/toggle.hpp"
#include "editor/diagram/unary.hpp"
#include "interaction.hpp"

// A node reads and writes each of its fields as text. A write that would not describe a valid node is refused and
// changes nothing; an accepted one changes the node and every part that shows it.

namespace {

  namespace fs = std::filesystem;
  namespace diagram = fluir::editor::diagram;
  namespace pt = fluir::pt;
  using fluir::editor::Field;
  using testutil::Interaction;

  pt::FunctionDecl& functionOf(pt::ParseTree& tree, fluir::ID id) {
    return std::get<pt::FunctionDecl>(tree.declarations.at(id));
  }

  template <typename T>
  T& nodeOf(pt::ParseTree& tree, fluir::ID function, fluir::ID id) {
    return std::get<T>(functionOf(tree, function).body.nodes.at(id));
  }

}  // namespace

class FieldsOnEachFixture : public testing::TestWithParam<fs::path> { };

INSTANTIATE_TEST_SUITE_P(Fixtures,
                         FieldsOnEachFixture,
                         testing::ValuesIn(testutil::readWriteFixtures(true)),
                         testutil::fixtureName);

TEST_P(FieldsOnEachFixture, EveryFieldPartShowsItsNodesText) {
  Interaction h(GetParam().string());

  for (const diagram::FieldText* part : testutil::all<diagram::FieldText>(h.scene())) {
    EXPECT_EQ(part->toPlainText().toStdString(), part->node().text(part->field())) << "node " << part->node().id();
  }
  for (const diagram::FieldView* part : testutil::all<diagram::FieldView>(h.scene())) {
    EXPECT_EQ(part->text().toStdString(), part->node().text(part->field())) << "node " << part->node().id();
  }
  for (const diagram::Toggle* part : testutil::all<diagram::Toggle>(h.scene())) {
    EXPECT_EQ(part->value() ? "true" : "false", part->node().text(part->field())) << "node " << part->node().id();
  }
}

TEST(Fields, AFunctionNameIsAnIdentifier) {
  Interaction h("read/simple_binary_expr.fl");
  auto& function = h.find<diagram::Function>(1);
  pt::ParseTree expected = h.tree();
  functionOf(expected, 1).name = "renamed";

  EXPECT_EQ(function.text({Field::Kind::Name}), "foo");
  EXPECT_FALSE(function.setText({Field::Kind::Name}, "1bad"));
  EXPECT_FALSE(function.setText({Field::Kind::Name}, ""));
  ASSERT_TRUE(function.setText({Field::Kind::Name}, "renamed"));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(Interaction::fieldOf<diagram::FieldText>(function, {Field::Kind::Name}).toPlainText(), "renamed");
}

TEST(Fields, AParameterNameIsAnIdentifier) {
  Interaction h("read/function_with_input_only.fl");
  auto& function = h.find<diagram::Function>(1);
  const Field name{Field::Kind::ParamName, 1};
  pt::ParseTree expected = h.tree();
  functionOf(expected, 1).input->parameters[1].name = "c";

  EXPECT_EQ(function.text(name), "b");
  EXPECT_FALSE(function.setText(name, "not valid"));
  ASSERT_TRUE(function.setText(name, "c"));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(Interaction::fieldOf<diagram::FieldText>(function, name).toPlainText(), "c");
}

TEST(Fields, AParameterTypeIsABuiltinType) {
  Interaction h("read/function_with_input_only.fl");
  auto& function = h.find<diagram::Function>(1);
  const Field type{Field::Kind::ParamType, 0};
  pt::ParseTree expected = h.tree();
  functionOf(expected, 1).input->parameters[0].typeName = "F64";

  EXPECT_EQ(function.text(type), "I32");
  EXPECT_FALSE(function.setText(type, "Widget"));
  ASSERT_TRUE(function.setText(type, "F64"));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(Interaction::fieldOf<diagram::FieldView>(function, type).text(), "F64");
}

TEST(Fields, AReturnTypeIsABuiltinType) {
  Interaction h("read/function_with_output_only.fl");
  auto& function = h.find<diagram::Function>(1);
  const Field type{Field::Kind::ReturnType};
  pt::ParseTree expected = h.tree();
  functionOf(expected, 1).output->ret->typeName = "I8";

  EXPECT_EQ(function.text(type), "F64");
  EXPECT_FALSE(function.setText(type, "f64"));
  ASSERT_TRUE(function.setText(type, "I8"));

  EXPECT_EQ(h.tree(), expected);
}

TEST(Fields, ACallTargetAndItsArgumentsAreIdentifiers) {
  Interaction h("read/function_call.fl");
  auto& call = h.find<diagram::Call>(3);
  const Field second{Field::Kind::Arg, 1};
  pt::ParseTree expected = h.tree();
  nodeOf<pt::Call>(expected, 1, 3).target = "sum";
  nodeOf<pt::Call>(expected, 1, 3).arguments[1].name = "rhs";

  EXPECT_EQ(call.text({Field::Kind::Target}), "add");
  EXPECT_EQ(call.text(second), "b");
  EXPECT_FALSE(call.setText({Field::Kind::Target}, "a-b"));
  EXPECT_FALSE(call.setText(second, ""));
  ASSERT_TRUE(call.setText({Field::Kind::Target}, "sum"));
  ASSERT_TRUE(call.setText(second, "rhs"));

  EXPECT_EQ(h.tree(), expected);
}

TEST(Fields, AnIntegerLiteralFitsItsType) {
  Interaction h("read/int_constants.fl");
  auto& constant = h.find<diagram::Constant>(1);
  const Field literal{Field::Kind::Literal};
  pt::ParseTree expected = h.tree();
  nodeOf<pt::Constant>(expected, 1, 1).value = fluir::literals_types::I8{42};

  EXPECT_EQ(constant.text(literal), "-5");
  EXPECT_FALSE(constant.setText(literal, "300")) << "past I8";
  EXPECT_FALSE(constant.setText(literal, "4.2"));
  EXPECT_FALSE(constant.setText(literal, "abc"));
  ASSERT_TRUE(constant.setText(literal, "42"));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(Interaction::fieldOf<diagram::FieldText>(constant, literal).toPlainText(), "42");
}

TEST(Fields, AFloatLiteralParses) {
  Interaction h("read/simple_binary_expr.fl");
  auto& constant = h.find<diagram::Constant>(2);
  const Field literal{Field::Kind::Literal};
  pt::ParseTree expected = h.tree();
  nodeOf<pt::Constant>(expected, 1, 2).value = fluir::literals_types::F64{2.5};

  EXPECT_FALSE(constant.setText(literal, "2.5.1"));
  ASSERT_TRUE(constant.setText(literal, "2.5"));

  EXPECT_EQ(h.tree(), expected);
}

TEST(Fields, ABoolIsTrueOrFalse) {
  Interaction h("read/read_boolean.fl");
  auto& constant = h.find<diagram::Constant>(1);
  const Field value{Field::Kind::Bool};
  pt::ParseTree expected = h.tree();
  nodeOf<pt::Constant>(expected, 1, 1).value = fluir::literals_types::BOOL{true};

  EXPECT_EQ(constant.text(value), "false");
  EXPECT_FALSE(constant.setText(value, "maybe"));
  ASSERT_TRUE(constant.setText(value, "true"));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_TRUE(Interaction::fieldOf<diagram::Toggle>(constant, value).value());
}

TEST(Fields, ABinaryTakesOnlyBinaryOperators) {
  Interaction h("read/simple_binary_expr.fl");
  auto& binary = h.find<diagram::Binary>(1);
  const Field op{Field::Kind::Operator};
  pt::ParseTree expected = h.tree();
  nodeOf<pt::Binary>(expected, 1, 1).op = fluir::Operator::STAR;

  EXPECT_EQ(binary.text(op), "+");
  EXPECT_FALSE(binary.setText(op, "!"));
  EXPECT_FALSE(binary.setText(op, "nope"));
  ASSERT_TRUE(binary.setText(op, "*"));

  EXPECT_EQ(h.tree(), expected);
  EXPECT_EQ(Interaction::fieldOf<diagram::FieldView>(binary, op).text(), "*");
}

TEST(Fields, AUnaryTakesOnlyUnaryOperators) {
  Interaction h("read/simple_unary_expr.fl");
  auto& unary = h.find<diagram::Unary>(7);
  const Field op{Field::Kind::Operator};

  EXPECT_FALSE(unary.setText(op, "*"));
  EXPECT_TRUE(unary.setText(op, "!"));
  EXPECT_EQ(unary.text(op), "!");
}

TEST(Fields, CommentTextIsAnything) {
  Interaction h("read/top_level_comment_only.fl");
  auto& comment = h.find<diagram::Comment>(1);
  const Field text{Field::Kind::Text};

  EXPECT_EQ(comment.text(text), "hello");
  ASSERT_TRUE(comment.setText(text, "two\nlines"));
  EXPECT_EQ(std::get<pt::Comment>(h.tree().declarations.at(1)).text, "two\nlines");
  ASSERT_TRUE(comment.setText(text, ""));
  EXPECT_EQ(std::get<pt::Comment>(h.tree().declarations.at(1)).text, "");
}
