#include "editor/view/graph_layout.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <variant>

#include "editor/core/graph_geometry.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/node_view.hpp"

namespace fluir::editor {
  namespace {
    // Grip sizes, in grid units.
    constexpr double DRAG_SIZE = 3;
    constexpr double DRAG_INSET = 1;
    constexpr double GRIP_THICKNESS = 0.8;
    constexpr double RESIZE_CORNER_SIZE = 3;
    // A terminal's hit square side, in grid units.
    constexpr double PORT_HIT_UNITS = 1;

    using Terminals = std::unordered_map<fluir::ID, TerminalSet>;

    // A draggable edge is a thick line lying just inside the rect's border; the whole line is the grip.
    Rect resizeEdgeX(const Rect& r, double unit) {
      return {r.x + r.w - GRIP_THICKNESS * unit, r.y, GRIP_THICKNESS * unit, r.h};
    }

    Rect resizeCorner(const Rect& r, double unit) {
      const double size = RESIZE_CORNER_SIZE * unit;
      return {r.x + r.w - size, r.y + r.h - size, size, size};
    }

    FullID childOf(const FullID& parent, fluir::ID id) {
      FullID out = parent;
      out.push_back(id);
      return out;
    }

    const FlowGraphLocation& locationOf(const pt::Node& node) {
      return std::visit([](const auto& n) -> const FlowGraphLocation& { return n.location; }, node);
    }

    fluir::ID idOf(const pt::Node& node) {
      return std::visit([](const auto& n) { return n.id; }, node);
    }

    // A bool constant draws a fixed-size toggle square, so there is nothing to widen: no resize grip.
    std::optional<Part> resizePart(const pt::Node& node) {
      if (std::holds_alternative<pt::Comment>(node)) {
        return Part::ResizeXY;
      }
      const auto* c = std::get_if<pt::Constant>(&node);
      if (c != nullptr && std::holds_alternative<literals_types::BOOL>(c->value)) {
        return std::nullopt;
      }
      return Part::ResizeX;
    }

    // A node's body, then its grips over it. `resize` is ResizeX (right edge), ResizeXY (corner) or none.
    void pushNodeBoxes(const FullID& path,
                       const Rect& rect,
                       const std::optional<Rect>& clip,
                       std::optional<Part> resize,
                       double unit,
                       std::vector<Box>& out) {
      out.push_back({path, Part::Body, rect, clip});
      if (resize) {
        const Rect grip = *resize == Part::ResizeXY ? resizeCorner(rect, unit) : resizeEdgeX(rect, unit);
        out.push_back({path, *resize, grip, clip});
      }
      out.push_back({path, Part::MoveGrip, moveGrip(rect, unit), clip});
    }

    void layoutBlock(const pt::Block& block,
                     const FullID& parent,
                     Vec2 origin,
                     const Rect& clip,
                     const EditorContext::Layout& layout,
                     Terminals terminals,
                     std::vector<Box>& out);

    // The conditional's body, its visible branch's nodes, then its chrome over them.
    void layoutConditional(const pt::Conditional& conditional,
                           const FullID& path,
                           const Rect& frame,
                           const std::optional<Rect>& clip,
                           const EditorContext::Layout& layout,
                           std::vector<Box>& out) {
      const double unit = layout.unitPx;
      out.push_back({path, Part::Body, frame, clip});

      // One frame, one header band, one visible branch: the branch is the frame under its header.
      const Vec2 origin = bodyOrigin(frame.topLeft(), layout.headerH());
      const Rect content{frame.x, origin.y, frame.w, frame.y + frame.h - origin.y};
      const FullID branchPath = childOf(path, THEN_BRANCH_ID);
      if (const std::optional<Rect> contentClip = clip ? intersect(*clip, content) : std::optional<Rect>{content}) {
        out.push_back({branchPath, Part::Branch, content, contentClip});
        layoutBlock(*conditional.thenScope, branchPath, origin, *contentClip, layout, Terminals{}, out);
      }

      // The frame's chrome paints, and so hits, over its branch.
      const Rect header{frame.x, frame.y, frame.w, layout.headerH()};
      out.push_back({path, Part::Frame, frame, clip});
      out.push_back({path, Part::ResizeXY, resizeCorner(frame, unit), clip});
      out.push_back({path, Part::MoveGrip, moveGrip(header, unit), clip});
    }

    // Nodes paint in (z, id) order with their grips; conduits paint after, from terminals resolved by id.
    // `terminals` is per block: node ids repeat across sibling blocks.
    void layoutBlock(const pt::Block& block,
                     const FullID& parent,
                     Vec2 origin,
                     const Rect& clip,
                     const EditorContext::Layout& layout,
                     Terminals terminals,
                     std::vector<Box>& out) {
      for (const pt::Node* node : sortedNodes(block)) {
        const FullID path = childOf(parent, idOf(*node));
        if (const auto* conditional = std::get_if<pt::Conditional>(node)) {
          layoutConditional(
            *conditional, path, atOrigin(origin, localRect(conditional->location, layout.unitPx)), clip, layout, out);
          continue;
        }
        const Rect rect = atOrigin(origin, localRect(locationOf(*node), layout.unitPx));
        pushNodeBoxes(path, rect, clip, resizePart(*node), layout.unitPx, out);
        terminals[idOf(*node)] = editor::terminals(*node, rect, layout);
      }

      // A dangling endpoint is a legitimate authoring state: it just draws no line.
      for (const pt::Conduit* conduit : sortedConduits(block)) {
        const auto source = terminals.find(conduit->input);
        if (source == terminals.end() || source->second.outputs.empty()) {
          continue;
        }
        const Vec2 from = source->second.outputs.front();
        for (const pt::Conduit::Output& target : conduit->children) {
          const auto sink = terminals.find(target.target);
          if (sink == terminals.end() || target.index < 0 ||
              static_cast<std::size_t>(target.index) >= sink->second.inputs.size()) {
            continue;
          }
          const Vec2 to = sink->second.inputs[static_cast<std::size_t>(target.index)];
          out.push_back(
            {childOf(parent, conduit->id), Part::Wire, Rect{from.x, from.y, to.x - from.x, to.y - from.y}, clip});
        }
      }
    }

    void layoutFunction(const pt::FunctionDecl& fn, const EditorContext::Layout& layout, std::vector<Box>& out) {
      const FullID path{fn.id};
      const double unit = layout.unitPx;
      const Rect frame = localRect(fn.location, unit);
      const Vec2 origin = bodyOrigin(frame.topLeft(), layout.headerH());
      const Rect clip{frame.x, origin.y, frame.w, frame.h - layout.headerH()};
      out.push_back({path, Part::Body, frame, std::nullopt});

      Terminals terminals;
      if (fn.input) {
        std::vector<const pt::FunctionDecl::Parameter*> params;
        for (const auto& param : fn.input->parameters) {
          params.push_back(&param);
        }
        std::ranges::sort(params, {}, &pt::FunctionDecl::Parameter::index);
        for (std::size_t row = 0; row < params.size(); ++row) {
          const Rect rail{
            origin.x, origin.y + static_cast<double>(row) * layout.railStep(), layout.paramW(), layout.railStep()};
          out.push_back({childOf(path, params[row]->id), Part::Rail, rail, clip});
          terminals[params[row]->id] = draw::anchors(fn, params[row]->id, rail);
        }
      }
      if (fn.output && fn.output->ret) {
        const Rect rail{origin.x + (fn.location.width - layout.returnInsetUnits) * unit,
                        origin.y,
                        layout.railStep(),
                        layout.railStep()};
        out.push_back({childOf(path, fn.output->ret->id), Part::Rail, rail, clip});
        terminals[fn.output->ret->id] = draw::anchors(fn, fn.output->ret->id, rail);
      }

      layoutBlock(fn.body, path, origin, clip, layout, std::move(terminals), out);

      // The frame's chrome paints, and so hits, over its body.
      const Rect header{frame.x, frame.y, frame.w, layout.headerH()};
      out.push_back({path, Part::Frame, frame, std::nullopt});
      out.push_back({path, Part::ResizeXY, resizeCorner(frame, unit), std::nullopt});
      out.push_back({path, Part::MoveGrip, moveGrip(header, unit), std::nullopt});
    }

    bool hittable(Part part) { return part != Part::Frame && part != Part::Rail && part != Part::Wire; }

    // A node's or rail's terminal anchors, given its box.
    TerminalSet terminalsOf(const pt::ParseTree& tree, const Box& box, const EditorContext::Layout& layout) {
      if (box.path.size() < 2) {
        return {};
      }
      if (box.part == Part::Body) {
        const pt::Node* node = nodeAt(tree, box.path);
        return node == nullptr ? TerminalSet{} : terminals(*node, box.world, layout);
      }
      const pt::FunctionDecl* fn = box.part == Part::Rail ? functionAt(tree, parentOf(box.path)) : nullptr;
      return fn == nullptr ? TerminalSet{} : draw::anchors(*fn, box.path.back(), box.world);
    }

  }  // namespace

  Rect moveGrip(const Rect& frame, double unit) {
    return {frame.x + frame.w - (DRAG_SIZE + DRAG_INSET) * unit,
            frame.y + DRAG_INSET * unit,
            DRAG_SIZE * unit,
            DRAG_SIZE * unit};
  }

  std::vector<Box> layoutGraph(const pt::ParseTree& tree, const EditorContext::Layout& layout) {
    std::vector<Box> out;
    for (const pt::Declaration* decl : sortedDeclarations(tree)) {
      if (const auto* fn = std::get_if<pt::FunctionDecl>(decl)) {
        layoutFunction(*fn, layout, out);
      } else if (const auto* comment = std::get_if<pt::Comment>(decl)) {
        pushNodeBoxes(FullID{comment->id},
                      localRect(comment->location, layout.unitPx),
                      std::nullopt,
                      Part::ResizeXY,
                      layout.unitPx,
                      out);
      }
    }
    return out;
  }

  const Box* hitAt(std::span<const Box> boxes, Vec2 world) {
    for (auto it = boxes.rbegin(); it != boxes.rend(); ++it) {
      if (hittable(it->part) && (!it->clip || it->clip->contains(world)) && it->world.contains(world)) {
        return &*it;
      }
    }
    return nullptr;
  }

  const Box* railAt(std::span<const Box> boxes, const FullID& fnPath, Vec2 world) {
    for (auto it = boxes.rbegin(); it != boxes.rend(); ++it) {
      if (it->part == Part::Rail && parentOf(it->path) == fnPath && (!it->clip || it->clip->contains(world)) &&
          it->world.contains(world)) {
        return &*it;
      }
    }
    return nullptr;
  }

  std::optional<TerminalHit> terminalAt(const pt::ParseTree& tree,
                                        std::span<const Box> boxes,
                                        Vec2 world,
                                        const EditorContext::Layout& layout) {
    const double side = PORT_HIT_UNITS * layout.unitPx;
    for (auto it = boxes.rbegin(); it != boxes.rend(); ++it) {
      if (it->clip && !it->clip->contains(world)) {
        continue;
      }
      const TerminalSet set = terminalsOf(tree, *it, layout);
      for (const bool output : {false, true}) {
        const std::vector<Vec2>& anchors = output ? set.outputs : set.inputs;
        for (std::size_t i = 0; i < anchors.size(); ++i) {
          if (dotRect(anchors[i], side).contains(world)) {
            return TerminalHit{it->path, output, static_cast<int>(i), anchors[i]};
          }
        }
      }
    }
    return std::nullopt;
  }

  Rect graphBounds(std::span<const Box> boxes) {
    bool any = false;
    double minX = 0, minY = 0, maxX = 0, maxY = 0;
    for (const Box& box : boxes) {
      // A function's body box is its frame.
      if (box.part != Part::Body || box.path.size() != 1) {
        continue;
      }
      const Rect& r = box.world;
      minX = any ? std::min(minX, r.x) : r.x;
      minY = any ? std::min(minY, r.y) : r.y;
      maxX = any ? std::max(maxX, r.x + r.w) : r.x + r.w;
      maxY = any ? std::max(maxY, r.y + r.h) : r.y + r.h;
      any = true;
    }
    return {minX, minY, maxX - minX, maxY - minY};
  }

}  // namespace fluir::editor
