#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/core/field.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/viewport.hpp"
#include "editor/view/draw/binary.hpp"
#include "editor/view/draw/call.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/constant.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/draw/unary.hpp"
#include "recording_renderer.hpp"

// Each kind names one label per editable field, inside what it labels, and draws that field's value there.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::Field;
  using fluir::editor::FieldLabel;
  using fluir::editor::Rect;
  using fluir::editor::Subview;
  using fluir::editor::Viewport;
  using testutil::DrawCall;
  using testutil::RecordingRenderer;
  using Kind = fluir::editor::Field::Kind;

  const EditorContext kCtx;
  const Rect kRect{10, 20, 120, 90};

  std::vector<Field> fieldsOf(const std::vector<FieldLabel>& labels) {
    std::vector<Field> out;
    for (const FieldLabel& label : labels) {
      out.push_back(label.field);
    }
    return out;
  }

  bool inside(const Rect& inner, const Rect& outer) {
    return inner.x >= outer.x && inner.y >= outer.y && inner.x + inner.w <= outer.x + outer.w &&
           inner.y + inner.h <= outer.y + outer.h;
  }

  void expectAllInside(const std::vector<FieldLabel>& labels, const Rect& outer) {
    for (const FieldLabel& label : labels) {
      EXPECT_TRUE(inside(label.rect, outer)) << "field kind " << static_cast<int>(label.field.kind);
    }
  }

  const FieldLabel& labelFor(const std::vector<FieldLabel>& labels, Field field) {
    const auto it = std::ranges::find(labels, field, &FieldLabel::field);
    EXPECT_NE(it, labels.end());
    return *it;
  }

  // An identity view: world px are screen px.
  Subview rootView(RecordingRenderer& r, const Viewport& viewport) {
    return Subview{viewport, Rect{0, 0, r.outputSize().x, r.outputSize().y}, r};
  }

  bool textDrawnIn(const std::vector<DrawCall>& calls, const std::string& text, const Rect& rect) {
    return std::ranges::any_of(
      calls, [&](const DrawCall& c) { return c.op == DrawCall::Op::Text && c.text == text && rect.contains(c.a); });
  }

  fluir::editor::et::Call makeCall() {
    return fluir::editor::et::Call{.id = 1,
                                   .location = {},
                                   .target = "foo",
                                   ._return = std::nullopt,
                                   .arguments = {{.name = "b", .index = 1}, {.name = "a", .index = 0}}};
  }

  fluir::editor::et::FunctionDecl makeFunction() {
    fluir::editor::et::FunctionDecl fn;
    fn.id = 1;
    fn.name = "main";
    fn.input = fluir::editor::et::FunctionDecl::InputBlock{
      .parameters = {{.id = 20, .index = 3, .name = "x", .typeName = "I32"}}};
    fn.output = fluir::editor::et::FunctionDecl::OutputBlock{
      .ret = fluir::editor::et::FunctionDecl::Return{.id = 25, .typeName = "I32"}};
    return fn;
  }

}  // namespace

TEST(Labels, BinaryAndUnaryLabelTheirOperator) {
  const auto binary =
    fluir::editor::draw::labels(fluir::editor::et::Binary{.id = 1, .op = fluir::Operator::PLUS}, kRect, kCtx.layout);
  const auto unary =
    fluir::editor::draw::labels(fluir::editor::et::Unary{.id = 1, .op = fluir::Operator::MINUS}, kRect, kCtx.layout);

  EXPECT_EQ(fieldsOf(binary), std::vector<Field>{{Kind::Operator}});
  EXPECT_EQ(fieldsOf(unary), std::vector<Field>{{Kind::Operator}});
  expectAllInside(binary, kRect);
  expectAllInside(unary, kRect);
}

TEST(Labels, CallLabelsItsTargetAndEachArgumentByIndex) {
  const auto labels = fluir::editor::draw::labels(makeCall(), kRect, kCtx.layout);

  EXPECT_EQ(fieldsOf(labels), (std::vector<Field>{{Kind::Target}, {Kind::Arg, 0}, {Kind::Arg, 1}}));
  expectAllInside(labels, kRect);
}

TEST(Labels, CallRowsDoNotOverlap) {
  const auto labels = fluir::editor::draw::labels(makeCall(), kRect, kCtx.layout);
  ASSERT_EQ(labels.size(), 3u);

  for (std::size_t i = 1; i < labels.size(); ++i) {
    EXPECT_LE(labels[i - 1].rect.y + labels[i - 1].rect.h, labels[i].rect.y);
  }
}

TEST(Labels, CallDrawsEachValueInsideItsLabel) {
  RecordingRenderer r;
  const Viewport viewport;
  const fluir::editor::et::Call call = makeCall();
  const auto labels = fluir::editor::draw::labels(call, kRect, kCtx.layout);

  fluir::editor::draw::draw(call, kRect, rootView(r, viewport), kCtx);

  EXPECT_TRUE(textDrawnIn(r.calls, "foo", labelFor(labels, {Kind::Target}).rect));
  EXPECT_TRUE(textDrawnIn(r.calls, "a", labelFor(labels, {Kind::Arg, 0}).rect));
  EXPECT_TRUE(textDrawnIn(r.calls, "b", labelFor(labels, {Kind::Arg, 1}).rect));
}

TEST(Labels, ConstantLabelsItsLiteral) {
  RecordingRenderer r;
  const Viewport viewport;
  const fluir::editor::et::Constant constant{.id = 1, .location = {}, .value = fluir::literals_types::I32{7}};
  const auto labels = fluir::editor::draw::labels(constant, kRect, kCtx.layout);

  fluir::editor::draw::draw(constant, kRect, rootView(r, viewport), kCtx);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::Literal}});
  expectAllInside(labels, kRect);
  EXPECT_TRUE(textDrawnIn(r.calls, "7", labels.front().rect));
}

TEST(Labels, BoolConstantLabelsItsToggle) {
  RecordingRenderer r;
  const Viewport viewport;
  const fluir::editor::et::Constant constant{.id = 1, .location = {}, .value = fluir::literals_types::BOOL{true}};
  const auto labels = fluir::editor::draw::labels(constant, kRect, kCtx.layout);

  fluir::editor::draw::draw(constant, kRect, rootView(r, viewport), kCtx);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::Bool}});
  expectAllInside(labels, kRect);
  const auto icons = testutil::opsOf(r.calls, DrawCall::Op::Icon);
  EXPECT_TRUE(std::ranges::any_of(icons, [&](const DrawCall& c) { return inside(c.rect, labels.front().rect); }));
}

TEST(Labels, CommentLabelsItsTextAndWrapsItThere) {
  RecordingRenderer r;
  const Viewport viewport;
  const fluir::editor::et::Comment comment{.id = 1, .location = {}, .text = "note"};
  const auto labels = fluir::editor::draw::labels(comment, kRect, kCtx.layout);

  fluir::editor::draw::draw(comment, kRect, rootView(r, viewport), kCtx);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::Text}});
  expectAllInside(labels, kRect);
  const auto wrapped = testutil::opsOf(r.calls, DrawCall::Op::TextWrapped);
  ASSERT_FALSE(wrapped.empty());
  EXPECT_TRUE(inside(wrapped.front().rect, labels.front().rect));
}

TEST(Labels, ConditionalHasNoLabels) {
  const fluir::editor::et::Conditional conditional{.id = 1,
                                                   .location = {},
                                                   .condition = {},
                                                   .inputs = {},
                                                   .outputs = {},
                                                   .thenScope = xyz::indirect<fluir::editor::et::Block>{},
                                                   .elseScope = xyz::indirect<fluir::editor::et::Block>{}};

  EXPECT_TRUE(fluir::editor::draw::labels(conditional, kRect, kCtx.layout).empty());
}

TEST(Labels, FunctionFrameLabelsItsNameInTheHeaderBand) {
  RecordingRenderer r;
  const Viewport viewport;
  const fluir::editor::et::FunctionDecl fn = makeFunction();
  const auto labels = fluir::editor::draw::labels(fn, kRect, kCtx.layout);

  fluir::editor::draw::drawFrame(fn, kRect, rootView(r, viewport), kCtx);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::Name}});
  expectAllInside(labels, Rect{kRect.x, kRect.y, kRect.w, kCtx.layout.headerH()});
  EXPECT_TRUE(textDrawnIn(r.calls, "main", labels.front().rect));
}

TEST(Labels, ParameterRailLabelsItsTypeAndNameByParameterIndex) {
  RecordingRenderer r;
  const Viewport viewport;
  const fluir::editor::et::FunctionDecl fn = makeFunction();
  const auto labels = fluir::editor::draw::labels(fn, 20, kRect, kCtx.layout);

  fluir::editor::draw::drawRail(fn, 20, kRect, rootView(r, viewport), kCtx);

  EXPECT_EQ(fieldsOf(labels), (std::vector<Field>{{Kind::ParamType, 3}, {Kind::ParamName, 3}}));
  expectAllInside(labels, kRect);
  const Rect type = labelFor(labels, {Kind::ParamType, 3}).rect;
  const Rect name = labelFor(labels, {Kind::ParamName, 3}).rect;
  EXPECT_LE(type.x + type.w, name.x) << "the type tag sits before the name";
  EXPECT_TRUE(textDrawnIn(r.calls, "x", name));
}

TEST(Labels, ReturnRailLabelsItsType) {
  const auto labels = fluir::editor::draw::labels(makeFunction(), 25, kRect, kCtx.layout);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::ReturnType}});
  expectAllInside(labels, kRect);
}
