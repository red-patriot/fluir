#include "editor/core/annotate.hpp"

#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/tree.hpp"

namespace {

  namespace pt = fluir::pt;
  namespace et = fluir::editor::et;
  using fluir::FlowGraphLocation;

  constexpr FlowGraphLocation kFnLoc{.x = 0, .y = 0, .z = 0, .width = 200, .height = 200};
  constexpr FlowGraphLocation kNodeLoc{.x = 3, .y = 4, .z = 1, .width = 5, .height = 6};
  constexpr FlowGraphLocation kCondLoc{.x = 10, .y = 12, .z = 2, .width = 100, .height = 80};

  // `indirect`'s default constructor is explicit, so a conditional is default-initialized and then filled.
  template <class A>
  pt::ConditionalT<A> makeConditional(fluir::ID id, pt::BlockT<A> thenScope, pt::BlockT<A> elseScope) {
    pt::ConditionalT<A> conditional;
    conditional.id = id;
    conditional.location = kCondLoc;
    conditional.condition = pt::BlockPort{.outerId = id + 100, .innerId = id + 101, .y = 10};
    conditional.inputs.emplace(id + 200, pt::BlockPort{.outerId = id + 200, .innerId = id + 201, .y = 20});
    conditional.outputs.emplace(id + 300, pt::BlockPort{.outerId = id + 300, .innerId = id + 301, .y = 30});
    *conditional.thenScope = std::move(thenScope);
    *conditional.elseScope = std::move(elseScope);
    return conditional;
  }

  // Every kind, every container and a conditional nested in a branch, so no field escapes the walk.
  template <class A>
  pt::ParseTreeT<A> makeTree() {
    pt::BlockT<A> innerThen;
    innerThen.nodes.emplace(18,
                            pt::ConstantT<A>{.id = 18, .location = kNodeLoc, .value = fluir::literals_types::I32{9}});

    pt::BlockT<A> thenScope;
    thenScope.nodes.emplace(16,
                            pt::ConstantT<A>{.id = 16, .location = kNodeLoc, .value = fluir::literals_types::F64{1.5}});
    thenScope.conduits.emplace(
      30, pt::Conduit{.id = 30, .input = 16, .index = 0, .children = {{.target = 16, .index = 1}}});

    pt::BlockT<A> elseScope;
    elseScope.nodes.emplace(17, makeConditional<A>(17, std::move(innerThen), pt::BlockT<A>{}));

    pt::FunctionDeclT<A> fn;
    fn.id = 1;
    fn.location = kFnLoc;
    fn.name = "main";
    fn.input = pt::FunctionInputBlock{.parameters = {{.id = 2, .index = 0, .name = "x", .typeName = "i32"},
                                                     {.id = 3, .index = 1, .name = "y", .typeName = "f64"}}};
    fn.output = pt::FunctionOutputBlock{.ret = pt::FunctionReturn{.id = 4, .typeName = "i32"}};
    fn.body.nodes.emplace(10, pt::ConstantT<A>{.id = 10, .location = kNodeLoc, .value = fluir::literals_types::I32{7}});
    fn.body.nodes.emplace(
      11, pt::BinaryT<A>{.id = 11, .location = kNodeLoc, .lhs = 10, .rhs = 10, .op = fluir::Operator::PLUS});
    fn.body.nodes.emplace(12, pt::UnaryT<A>{.id = 12, .location = kNodeLoc, .lhs = 11, .op = fluir::Operator::BANG});
    fn.body.nodes.emplace(13,
                          pt::CallT<A>{.id = 13,
                                       .location = kNodeLoc,
                                       .target = "g",
                                       ._return = pt::CallReturn{},
                                       .arguments = {{.name = "a", .index = 0}, {.name = "b", .index = 1}}});
    fn.body.nodes.emplace(14, pt::CommentT<A>{.id = 14, .location = kNodeLoc, .text = "inner"});
    fn.body.nodes.emplace(15, makeConditional<A>(15, std::move(thenScope), std::move(elseScope)));
    fn.body.conduits.emplace(31,
                             pt::Conduit{.id = 31, .input = 10, .index = 0, .children = {{.target = 11, .index = 0}}});

    pt::ParseTreeT<A> tree;
    tree.header = pt::Header{.version = {.major = 0, .minor = 1, .patch = 3}};
    tree.declarations.emplace(1, std::move(fn));
    tree.declarations.emplace(5, pt::CommentT<A>{.id = 5, .location = kNodeLoc, .text = "top"});
    return tree;
  }

  const et::Conditional& conditionalAt(const et::Block& block, fluir::ID id) {
    return std::get<et::Conditional>(block.nodes.at(id));
  }

  // The empty-annotation claim: `annotation` must cost no storage in the compiler's instantiation.
  struct ConstantTwin {
    fluir::ID id;
    FlowGraphLocation location;
    pt::Literal value;
  };
  static_assert(sizeof(pt::Constant) == sizeof(ConstantTwin));

  TEST(Annotate, CopiesEveryField) {
    EXPECT_EQ(fluir::editor::annotate(makeTree<pt::NoAnnotations>()), makeTree<et::Annotations>());
  }

  TEST(Annotate, AnnotationsComeOutDefault) {
    const et::ParseTree tree = fluir::editor::annotate(makeTree<pt::NoAnnotations>());
    const et::Block& body = std::get<et::FunctionDecl>(tree.declarations.at(1)).body;
    const et::Conditional& outer = conditionalAt(body, 15);
    EXPECT_EQ(outer.annotation.shownBranch, fluir::editor::THEN_BRANCH_ID);
    EXPECT_EQ(conditionalAt(*outer.elseScope, 17).annotation.shownBranch, fluir::editor::THEN_BRANCH_ID);
  }

  TEST(Annotate, AnAnnotationIsNotPartOfTheParsedShape) {
    et::ParseTree annotated = fluir::editor::annotate(makeTree<pt::NoAnnotations>());
    et::Block& body = std::get<et::FunctionDecl>(annotated.declarations.at(1)).body;
    std::get<et::Conditional>(body.nodes.at(15)).annotation.shownBranch = fluir::editor::ELSE_BRANCH_ID;
    EXPECT_NE(annotated, makeTree<et::Annotations>());
  }

}  // namespace
