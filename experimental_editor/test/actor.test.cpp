#include "editor/actors/actor.hpp"

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "compiler/models/operator.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/core/geometry.hpp"

namespace {

  using fluir::FlowGraphLocation;
  using fluir::ID;
  using fluir::Operator;
  using fluir::editor::Actor;
  using fluir::editor::BinaryActor;
  using fluir::editor::CallActor;
  using fluir::editor::ConstantActor;
  using fluir::editor::Rect;
  using fluir::editor::UnaryActor;
  using fluir::editor::Vec2;

  // Geometry fields are irrelevant to click dispatch; z is unused.
  const FlowGraphLocation kLoc{.x = 0, .y = 0, .z = 0, .width = 0, .height = 0};

  fluir::pt::Binary makeBinary() {
    fluir::pt::Binary binary;
    binary.id = 1;
    binary.location = kLoc;
    binary.lhs = 2;
    binary.rhs = 3;
    binary.op = Operator::PLUS;
    return binary;
  }

  fluir::pt::Unary makeUnary() {
    fluir::pt::Unary unary;
    unary.id = 4;
    unary.location = kLoc;
    unary.lhs = 5;
    unary.op = Operator::MINUS;
    return unary;
  }

  fluir::pt::Constant makeConstant() {
    fluir::pt::Constant constant;
    constant.id = 6;
    constant.location = kLoc;
    constant.value = fluir::literals_types::I32{42};
    return constant;
  }

  fluir::pt::Call makeCall() {
    fluir::pt::Call call;
    call.id = 7;
    call.location = kLoc;
    call.target = "add";
    return call;
  }

}  // namespace

TEST(Actor, BinaryActorOnClickSummarizesOperator) {
  BinaryActor actor(makeBinary(), Rect{0, 0, 10, 10});
  actor.onClick(Vec2{1, 1});

  EXPECT_NE(actor.lastClickSummary().find("+"), std::string::npos);
}

TEST(Actor, UnaryActorOnClickSummarizesOperator) {
  UnaryActor actor(makeUnary(), Rect{0, 0, 10, 10});
  actor.onClick(Vec2{1, 1});

  EXPECT_NE(actor.lastClickSummary().find("-"), std::string::npos);
}

TEST(Actor, ConstantActorOnClickSummarizesValue) {
  ConstantActor actor(makeConstant(), Rect{0, 0, 10, 10});
  actor.onClick(Vec2{1, 1});

  EXPECT_NE(actor.lastClickSummary().find("42"), std::string::npos);
}

TEST(Actor, CallActorOnClickSummarizesTarget) {
  CallActor actor(makeCall(), Rect{0, 0, 10, 10});
  actor.onClick(Vec2{1, 1});

  EXPECT_NE(actor.lastClickSummary().find("add"), std::string::npos);
}

// Virtual dispatch smoke test: each concrete actor is reachable and produces
// its own type-specific summary when clicked through a base `Actor*`, not
// just when clicked directly on the concrete type.
TEST(Actor, VirtualDispatchReachesEachOverride) {
  std::unique_ptr<Actor> binary = std::make_unique<BinaryActor>(makeBinary(), Rect{0, 0, 10, 10});
  std::unique_ptr<Actor> unary = std::make_unique<UnaryActor>(makeUnary(), Rect{0, 0, 10, 10});
  std::unique_ptr<Actor> constant = std::make_unique<ConstantActor>(makeConstant(), Rect{0, 0, 10, 10});
  std::unique_ptr<Actor> call = std::make_unique<CallActor>(makeCall(), Rect{0, 0, 10, 10});

  binary->onClick(Vec2{1, 1});
  unary->onClick(Vec2{1, 1});
  constant->onClick(Vec2{1, 1});
  call->onClick(Vec2{1, 1});

  EXPECT_NE(static_cast<BinaryActor&>(*binary).lastClickSummary().find("+"), std::string::npos);
  EXPECT_NE(static_cast<UnaryActor&>(*unary).lastClickSummary().find("-"), std::string::npos);
  EXPECT_NE(static_cast<ConstantActor&>(*constant).lastClickSummary().find("42"), std::string::npos);
  EXPECT_NE(static_cast<CallActor&>(*call).lastClickSummary().find("add"), std::string::npos);
}

TEST(Actor, IdAndBoundsReturnConstructionValues) {
  const Rect bounds{10, 20, 30, 40};
  BinaryActor actor(makeBinary(), bounds);

  EXPECT_EQ(actor.id(), 1u);
  EXPECT_EQ(actor.bounds(), bounds);
}
