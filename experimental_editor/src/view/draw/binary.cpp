#include "editor/view/draw/binary.hpp"

#include "compiler/models/operator.hpp"

namespace fluir::editor::draw {
  namespace {

    // In grid units. A node's height follows its content, so it is unbounded below.
    constexpr Limits<Vec2i> SIZE_LIMITS{.lower = Vec2i{4, 0}, .upper = Vec2i{1000, 1000}};

  }  // namespace

  TerminalSet anchors(const et::Binary&, const Rect& r, const EditorContext::Layout&) {
    return {edgeAnchors(r.x, r, 2), edgeAnchors(r.x + r.w, r, 1)};
  }

  Color color(const et::Binary&, const EditorContext::Theme& theme) { return theme.operatorNode; }

  Limits<Vec2i> sizeLimits(const et::Binary&) { return SIZE_LIMITS; }

  std::optional<Part> resizePart(const et::Binary&) { return Part::ResizeX; }

  std::vector<FieldLabel> labels(const et::Binary&, const Rect& world, const EditorContext::Layout&) {
    return {{{Field::Kind::Operator}, world}};
  }

}  // namespace fluir::editor::draw
