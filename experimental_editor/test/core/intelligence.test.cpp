#include "editor/core/intelligence.hpp"

#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/choices.hpp"
#include "editor/core/identifier.hpp"
#include "editor/core/scope_path.hpp"

// Intelligence offers tree prototypes: what may go at the top level, and what may go inside a body of a given tree.

namespace {

  namespace pt = fluir::pt;
  using fluir::editor::Completion;

  using fluir::editor::Intelligence;

  const Intelligence kIntelligence;

  template <typename Kind, typename T>
  std::vector<Kind> prototypesOf(const std::vector<Completion<T>>& completions) {
    std::vector<Kind> out;
    for (const Completion<T>& completion : completions) {
      if (const auto* kind = std::get_if<Kind>(&completion.prototype)) {
        out.push_back(*kind);
      }
    }
    return out;
  }

  template <typename T>
  void expectDistinctLabels(const std::vector<Completion<T>>& completions) {
    std::set<std::string> labels;
    for (const Completion<T>& completion : completions) {
      EXPECT_TRUE(labels.insert(completion.label).second) << completion.label;
      EXPECT_FALSE(completion.label.empty());
    }
  }

  pt::FunctionDecl function(fluir::ID id,
                            std::string name,
                            std::vector<pt::FunctionParameter> parameters,
                            std::optional<std::string> returnType) {
    pt::FunctionDecl fn{.id = id, .location = {}, .name = std::move(name), .body = {}, .input = {}, .output = {}};
    if (!parameters.empty()) {
      fn.input = pt::FunctionInputBlock{.parameters = std::move(parameters)};
    }
    if (returnType) {
      fn.output = pt::FunctionOutputBlock{.ret = pt::FunctionReturn{.id = 99, .typeName = *returnType}};
    }
    return fn;
  }

  // A tree whose function 1 holds conditional 2.
  pt::ParseTree treeWithBody() {
    pt::FunctionDecl main = function(1, "main", {}, std::nullopt);
    main.body.nodes.emplace(2,
                            pt::Conditional{.id = 2,
                                            .location = {},
                                            .condition = {.innerId = 1, .y = 10},
                                            .inputs = {},
                                            .outputs = {},
                                            .thenScope = xyz::indirect<pt::Block>{},
                                            .elseScope = xyz::indirect<pt::Block>{}});
    pt::ParseTree tree;
    tree.declarations.emplace(1, std::move(main));
    return tree;
  }

  std::vector<Completion<pt::Node>> bodyOf(const pt::ParseTree& tree = treeWithBody()) {
    return kIntelligence.completionsAt(tree, {1});
  }

  pt::Call callTo(const std::vector<Completion<pt::Node>>& completions, const std::string& target) {
    for (const pt::Call& call : prototypesOf<pt::Call>(completions)) {
      if (call.target == target) {
        return call;
      }
    }
    ADD_FAILURE() << "no call to " << target;
    return {};
  }

}  // namespace

TEST(Intelligence, TheTopLevelOffersAnEmptyFunctionAndAComment) {
  const auto completions = kIntelligence.topLevelCompletions({});

  const auto functions = prototypesOf<pt::FunctionDecl>(completions);
  ASSERT_EQ(functions.size(), 1U);
  EXPECT_TRUE(fluir::editor::isValidIdentifier(functions.front().name));
  EXPECT_EQ(functions.front().body, pt::Block{});
  EXPECT_FALSE(functions.front().input);
  EXPECT_FALSE(functions.front().output);
  EXPECT_EQ(prototypesOf<pt::Comment>(completions).size(), 1U);
  EXPECT_EQ(completions.size(), 2U);
  expectDistinctLabels(completions);
}

TEST(Intelligence, ABodyOffersEveryOperator) {
  const auto completions = bodyOf();

  std::vector<std::string> binary;
  for (const pt::Binary& node : prototypesOf<pt::Binary>(completions)) {
    binary.emplace_back(fluir::stringify(node.op));
  }
  std::vector<std::string> unary;
  for (const pt::Unary& node : prototypesOf<pt::Unary>(completions)) {
    unary.emplace_back(fluir::stringify(node.op));
  }

  EXPECT_EQ(binary, fluir::editor::binaryOperators());
  EXPECT_EQ(unary, fluir::editor::unaryOperators());
}

TEST(Intelligence, ABodyOffersAConstantOfEachLiteralType) {
  std::set<std::size_t> types;
  for (const pt::Constant& constant : prototypesOf<pt::Constant>(bodyOf())) {
    EXPECT_TRUE(types.insert(constant.value.index()).second) << "type " << constant.value.index();
  }

  EXPECT_EQ(types.size(), std::variant_size_v<fluir::literals_types::Literal>);
}

TEST(Intelligence, ABodyOffersACommentAndAnEmptyConditional) {
  const auto completions = bodyOf();

  EXPECT_EQ(prototypesOf<pt::Comment>(completions).size(), 1U);
  const auto conditionals = prototypesOf<pt::Conditional>(completions);
  ASSERT_EQ(conditionals.size(), 1U);
  EXPECT_TRUE(conditionals.front().inputs.empty());
  EXPECT_TRUE(conditionals.front().outputs.empty());
  EXPECT_EQ(*conditionals.front().thenScope, pt::Block{});
  EXPECT_EQ(*conditionals.front().elseScope, pt::Block{});
}

TEST(Intelligence, ABodyOffersACallToEachFunctionMatchingItsSignature) {
  pt::ParseTree tree;
  tree.declarations.emplace(
    1,
    function(1, "add", {{.id = 2, .name = "a", .typeName = "I32"}, {.id = 3, .name = "b", .typeName = "I32"}}, "I32"));
  tree.declarations.emplace(4, function(4, "noop", {}, std::nullopt));

  const auto completions = bodyOf(tree);

  const pt::Call add = callTo(completions, "add");
  EXPECT_EQ(add.arguments, (pt::Call::Arguments{{.name = "a", .index = 0}, {.name = "b", .index = 1}}));
  EXPECT_TRUE(add._return);
  const pt::Call noop = callTo(completions, "noop");
  EXPECT_TRUE(noop.arguments.empty());
  EXPECT_FALSE(noop._return);
  expectDistinctLabels(completions);
}

TEST(Intelligence, ABodyOffersPrint) {
  const pt::Call print = callTo(bodyOf(), "print");

  EXPECT_EQ(print.arguments.size(), 1U);
  EXPECT_FALSE(print._return);
}

TEST(Intelligence, EachBranchOffersWhatABodyOffers) {
  const pt::ParseTree tree = treeWithBody();
  const auto labels = [](const std::vector<Completion<pt::Node>>& completions) {
    std::vector<std::string> out;
    for (const auto& completion : completions) {
      out.push_back(completion.label);
    }
    return out;
  };

  ASSERT_FALSE(bodyOf(tree).empty());
  EXPECT_EQ(labels(kIntelligence.completionsAt(tree, {1, 2, fluir::editor::THEN_BRANCH_ID})), labels(bodyOf(tree)));
  EXPECT_EQ(labels(kIntelligence.completionsAt(tree, {1, 2, fluir::editor::ELSE_BRANCH_ID})), labels(bodyOf(tree)));
}

TEST(Intelligence, AScopeThatNamesNoBodyOffersNothing) {
  const pt::ParseTree tree = treeWithBody();

  EXPECT_TRUE(kIntelligence.completionsAt(tree, {}).empty());
  EXPECT_TRUE(kIntelligence.completionsAt(tree, {99}).empty());
  EXPECT_TRUE(kIntelligence.completionsAt(tree, {1, 2}).empty());
}
