#include <algorithm>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/field.hpp"
#include "editor/scene/item_style.hpp"
#include "editor/view/draw/binary.hpp"
#include "editor/view/draw/call.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/constant.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/draw/unary.hpp"

// Each kind names one label per editable field, inside what it labels.

namespace {

  using fluir::editor::EditorContext;
  using fluir::editor::Field;
  using fluir::editor::FieldLabel;
  using fluir::editor::Rect;
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

  fluir::pt::Call makeCall() {
    return fluir::pt::Call{.id = 1,
                           .location = {},
                           .target = "foo",
                           ._return = std::nullopt,
                           .arguments = {{.name = "b", .index = 1}, {.name = "a", .index = 0}}};
  }

  fluir::editor::draw::Signature makeFunction() {
    return {.name = "main",
            .input = fluir::pt::FunctionInputBlock{.parameters = {{.id = 19, .name = "w", .typeName = "I32"},
                                                                  {.id = 20, .name = "x", .typeName = "I32"}}},
            .output = fluir::pt::FunctionOutputBlock{.ret = fluir::pt::FunctionReturn{.id = 25, .typeName = "I32"}}};
  }

}  // namespace

TEST(Labels, BinaryAndUnaryLabelTheirOperator) {
  const auto binary =
    fluir::editor::draw::labels(fluir::pt::Binary{.id = 1, .op = fluir::Operator::PLUS}, kRect, kCtx.layout);
  const auto unary =
    fluir::editor::draw::labels(fluir::pt::Unary{.id = 1, .op = fluir::Operator::MINUS}, kRect, kCtx.layout);

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

TEST(Labels, ConstantLabelsItsLiteral) {
  const fluir::pt::Constant constant{.id = 1, .location = {}, .value = fluir::literals_types::I32{7}};
  const auto labels = fluir::editor::draw::labels(constant, kRect, kCtx.layout);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::Literal}});
  expectAllInside(labels, kRect);
}

TEST(Labels, BoolConstantLabelsItsToggle) {
  const fluir::pt::Constant constant{.id = 1, .location = {}, .value = fluir::literals_types::BOOL{true}};
  const auto labels = fluir::editor::draw::labels(constant, kRect, kCtx.layout);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::Bool}});
  expectAllInside(labels, kRect);
}

TEST(Labels, CommentLabelsItsText) {
  const fluir::pt::Comment comment{.id = 1, .location = {}, .text = "note"};
  const auto labels = fluir::editor::draw::labels(comment, kRect, kCtx.layout);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::Text}});
  expectAllInside(labels, kRect);
}

TEST(Labels, ConditionalHasNoLabels) {
  const fluir::editor::draw::Ports conditional{};

  EXPECT_TRUE(fluir::editor::draw::labels(conditional, kRect, kCtx.layout).empty());
}

TEST(Labels, FunctionFrameLabelsItsNameInTheHeaderBand) {
  const fluir::editor::draw::Signature fn = makeFunction();
  const auto labels = fluir::editor::draw::labels(fn, kRect, kCtx.layout);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::Name}});
  expectAllInside(labels, Rect{kRect.x, kRect.y, kRect.w, kCtx.layout.headerH()});
}

TEST(Labels, ParameterLabelsItsTypeAndNameByItsPosition) {
  const fluir::editor::draw::Signature fn = makeFunction();
  const auto labels = fluir::editor::draw::labels(fn, 20, kRect, kCtx.layout);

  ASSERT_EQ(fieldsOf(labels), (std::vector<Field>{{Kind::ParamType, 1}, {Kind::ParamName, 1}}));
  expectAllInside(labels, kRect);
  const Rect type = labelFor(labels, {Kind::ParamType, 1}).rect;
  const Rect name = labelFor(labels, {Kind::ParamName, 1}).rect;
  EXPECT_LE(type.x + type.w, name.x) << "the type tag sits before the name";
}

TEST(Labels, ReturnRailLabelsItsType) {
  const auto labels = fluir::editor::draw::labels(makeFunction(), 25, kRect, kCtx.layout);

  EXPECT_EQ(fieldsOf(labels), std::vector<Field>{{Kind::ReturnType}});
  expectAllInside(labels, kRect);
}

TEST(Labels, ATypeIsATagAndEveryOtherFieldIsText) {
  using fluir::editor::FieldLook;
  const fluir::editor::draw::Signature fn = makeFunction();
  const auto parameter = fluir::editor::draw::labels(fn, 20, kRect, kCtx.layout);
  const auto ret = fluir::editor::draw::labels(fn, 25, kRect, kCtx.layout);
  const auto header = fluir::editor::draw::labels(fn, kRect, kCtx.layout);
  const auto binary =
    fluir::editor::draw::labels(fluir::pt::Binary{.id = 1, .op = fluir::Operator::PLUS}, kRect, kCtx.layout);
  ASSERT_EQ(parameter.size(), 2U);
  ASSERT_EQ(ret.size(), 1U);

  EXPECT_EQ(labelFor(parameter, {Kind::ParamType, 1}).look, FieldLook::Tag);
  EXPECT_EQ(labelFor(parameter, {Kind::ParamName, 1}).look, FieldLook::Text);
  EXPECT_EQ(labelFor(ret, {Kind::ReturnType}).look, FieldLook::Tag);
  EXPECT_EQ(labelFor(header, {Kind::Name}).look, FieldLook::Text);
  EXPECT_EQ(labelFor(binary, {Kind::Operator}).look, FieldLook::Text);
}

TEST(Labels, ATypeTagIsWideEnoughForItsWholeText) {
  QFontDatabase::addApplicationFont(":/fonts/DejaVuSansMono.ttf");
  const fluir::editor::draw::Signature fn = makeFunction();
  const auto parameter = fluir::editor::draw::labels(fn, 20, kRect, kCtx.layout);
  const auto ret = fluir::editor::draw::labels(fn, 25, kRect, kCtx.layout);
  const double needed =
    QFontMetricsF(fluir::editor::style::tagFont()).horizontalAdvance("I32") + 2 * kCtx.layout.textPad;

  EXPECT_GE(labelFor(parameter, {Kind::ParamType, 1}).rect.w, needed);
  EXPECT_GE(labelFor(ret, {Kind::ReturnType}).rect.w, needed);
}
