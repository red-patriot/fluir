#include "editor/core/fields.hpp"

#include <gtest/gtest.h>

#include "editor/core/tree.hpp"

// read gives a text field's value; write's edit, once executed, makes read return the new text.

namespace {

  using fluir::FullID;
  using fluir::editor::Field;
  using Kind = fluir::editor::Field::Kind;
  namespace et = fluir::editor::et;
  namespace fields = fluir::editor::fields;

  // Function 1 "main" (param x at index 0, returns I32) holds constant 2, call 3 foo(a), comment 4 and binary 5;
  // comment 6 is top level.
  et::ParseTree makeTree() {
    et::FunctionDecl fn{.id = 1, .location = {}, .name = "main", .body = {}, .input = {}, .output = {}};
    fn.input = et::FunctionDecl::InputBlock{.parameters = {{.id = 20, .index = 0, .name = "x", .typeName = "I32"}}};
    fn.output = et::FunctionDecl::OutputBlock{.ret = et::FunctionDecl::Return{.id = 25, .typeName = "I32"}};
    fn.body.nodes.emplace(2, et::Constant{.id = 2, .location = {}, .value = fluir::literals_types::I32{7}});
    fn.body.nodes.emplace(
      3, et::Call{.id = 3, .location = {}, .target = "foo", ._return = {}, .arguments = {{.name = "a", .index = 0}}});
    fn.body.nodes.emplace(4, et::Comment{.id = 4, .location = {}, .text = "inner"});
    fn.body.nodes.emplace(5, et::Binary{.id = 5, .location = {}, .op = fluir::Operator::PLUS});
    et::ParseTree tree;
    tree.declarations.emplace(1, std::move(fn));
    tree.declarations.emplace(6, et::Comment{.id = 6, .location = {}, .text = "outer"});
    return tree;
  }

  const FullID kFn{1};
  const FullID kConstant{1, 2};
  const FullID kCall{1, 3};
  const FullID kComment{1, 4};
  const FullID kBinary{1, 5};
  const FullID kTopComment{6};

  // Writes `text`, executes the edit and returns what read then gives.
  std::optional<std::string> roundTrip(const FullID& path, Field field, const std::string& text) {
    et::ParseTree tree = makeTree();
    auto edit = fields::write(tree, path, field, text);
    if (!edit || !*edit || !(*edit)->execute(tree)) {
      return std::nullopt;
    }
    return fields::read(tree, path, field);
  }

  bool unchanged(const FullID& path, Field field, const std::string& text) {
    const auto edit = fields::write(makeTree(), path, field, text);
    return edit && *edit == nullptr;
  }

  bool rejected(const FullID& path, Field field, const std::string& text) {
    return !fields::write(makeTree(), path, field, text);
  }

}  // namespace

TEST(Fields, ReadGivesEachTextField) {
  const et::ParseTree tree = makeTree();

  EXPECT_EQ(fields::read(tree, kFn, {Kind::Name}), "main");
  EXPECT_EQ(fields::read(tree, kFn, {Kind::ParamName, 0}), "x");
  EXPECT_EQ(fields::read(tree, kCall, {Kind::Target}), "foo");
  EXPECT_EQ(fields::read(tree, kCall, {Kind::Arg, 0}), "a");
  EXPECT_EQ(fields::read(tree, kConstant, {Kind::Literal}), "7");
  EXPECT_EQ(fields::read(tree, kComment, {Kind::Text}), "inner");
  EXPECT_EQ(fields::read(tree, kTopComment, {Kind::Text}), "outer");
}

TEST(Fields, ReadGivesEachChoiceField) {
  const et::ParseTree tree = makeTree();

  EXPECT_EQ(fields::read(tree, kBinary, {Kind::Operator}), "+");
  EXPECT_EQ(fields::read(tree, kFn, {Kind::ParamType, 0}), "I32");
  EXPECT_EQ(fields::read(tree, kFn, {Kind::ReturnType}), "I32");
}

TEST(Fields, ReadOfAMissingFieldIsNullopt) {
  const et::ParseTree tree = makeTree();

  EXPECT_EQ(fields::read(tree, {1, 99}, {Kind::Literal}), std::nullopt) << "unknown path";
  EXPECT_EQ(fields::read(tree, kCall, {Kind::Name}), std::nullopt) << "a call has no name";
  EXPECT_EQ(fields::read(tree, kCall, {Kind::Arg, 5}), std::nullopt) << "no argument at index 5";
  EXPECT_EQ(fields::read(tree, kFn, {Kind::ParamName, 5}), std::nullopt) << "no parameter at index 5";
  EXPECT_EQ(fields::read(tree, kConstant, {Kind::Operator}), std::nullopt) << "a constant has no operator";
  EXPECT_EQ(fields::read(tree, kFn, {Kind::ParamType, 5}), std::nullopt) << "no parameter at index 5";
  EXPECT_EQ(fields::read(tree, kCall, {Kind::ReturnType}), std::nullopt) << "a call has no return rail";
}

TEST(Fields, WriteEditsEachTextField) {
  EXPECT_EQ(roundTrip(kFn, {Kind::Name}, "go"), "go");
  EXPECT_EQ(roundTrip(kFn, {Kind::ParamName, 0}, "y"), "y");
  EXPECT_EQ(roundTrip(kCall, {Kind::Target}, "bar"), "bar");
  EXPECT_EQ(roundTrip(kCall, {Kind::Arg, 0}, "b"), "b");
  EXPECT_EQ(roundTrip(kConstant, {Kind::Literal}, "42"), "42");
  EXPECT_EQ(roundTrip(kComment, {Kind::Text}, "any text!"), "any text!");
  EXPECT_EQ(roundTrip(kTopComment, {Kind::Text}, ""), "");
}

TEST(Fields, WriteEditsEachChoiceField) {
  EXPECT_EQ(roundTrip(kBinary, {Kind::Operator}, "-"), "-");
  EXPECT_EQ(roundTrip(kFn, {Kind::ParamType, 0}, "F64"), "F64");
  EXPECT_EQ(roundTrip(kFn, {Kind::ReturnType}, "BOOL"), "BOOL");
}

TEST(Fields, WriteOfTheCurrentValueIsUnchanged) {
  EXPECT_TRUE(unchanged(kFn, {Kind::Name}, "main"));
  EXPECT_TRUE(unchanged(kFn, {Kind::ParamName, 0}, "x"));
  EXPECT_TRUE(unchanged(kCall, {Kind::Target}, "foo"));
  EXPECT_TRUE(unchanged(kCall, {Kind::Arg, 0}, "a"));
  EXPECT_TRUE(unchanged(kConstant, {Kind::Literal}, "7"));
  EXPECT_TRUE(unchanged(kComment, {Kind::Text}, "inner"));
  EXPECT_TRUE(unchanged(kBinary, {Kind::Operator}, "+"));
  EXPECT_TRUE(unchanged(kFn, {Kind::ParamType, 0}, "I32"));
  EXPECT_TRUE(unchanged(kFn, {Kind::ReturnType}, "I32"));
}

TEST(Fields, WriteRejectsInvalidText) {
  EXPECT_TRUE(rejected(kFn, {Kind::Name}, "not an id"));
  EXPECT_TRUE(rejected(kFn, {Kind::ParamName, 0}, "1x"));
  EXPECT_TRUE(rejected(kCall, {Kind::Target}, ""));
  EXPECT_TRUE(rejected(kCall, {Kind::Arg, 0}, "a b"));
  EXPECT_TRUE(rejected(kConstant, {Kind::Literal}, "seven"));
  EXPECT_TRUE(rejected(kBinary, {Kind::Operator}, "%%"));
  EXPECT_TRUE(rejected(kFn, {Kind::ParamType, 0}, "not a type"));
}

TEST(Fields, WriteOfAMissingFieldIsRejected) {
  EXPECT_TRUE(rejected({1, 99}, {Kind::Literal}, "1"));
  EXPECT_TRUE(rejected(kCall, {Kind::Name}, "bar"));
  EXPECT_TRUE(rejected(kCall, {Kind::Arg, 5}, "b"));
  EXPECT_TRUE(rejected(kConstant, {Kind::Operator}, "-"));
  EXPECT_TRUE(rejected(kCall, {Kind::ReturnType}, "I32"));
}
