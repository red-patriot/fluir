#include "editor/view/draw/binary.hpp"

#include "compiler/models/operator.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units. A node's height follows its content, so it is unbounded below.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{7, 0}, .upper = Vec2i{1000, 1000}};
    constexpr Vec2i DEFAULT_SIZE{8, 5};

  }  // namespace

  TerminalSet anchors(const pt::Binary&, const Rect& r, const EditorContext::Layout&) {
    return {edgeAnchors(r.x, r, 2), edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const pt::Binary&, const EditorContext::Theme& theme) { return theme.operatorNode; }

  Vec2i defaultSize(const pt::Binary&) { return DEFAULT_SIZE; }

  Limits<Vec2i> sizeLimits(const pt::Binary&) { return SIZE_LIMITS; }

  std::optional<ResizeAxes> resizePart(const pt::Binary&) { return ResizeAxes::X; }

  std::vector<FieldLabel> labels(const pt::Binary&, const Rect& world, const EditorContext::Layout&) {
    return {{{Field::Kind::Operator}, world}};
  }

}  // namespace fluir::editor::draw
