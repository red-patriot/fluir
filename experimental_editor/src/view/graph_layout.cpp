#include "editor/view/graph_layout.hpp"

#include <algorithm>
#include <cstddef>
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
    constexpr double RESIZE_BAR_WIDTH = 1;
    constexpr double RESIZE_CORNER_SIZE = 3;
    // A port's hit square side, in grid units.
    constexpr double PORT_HIT_UNITS = 1;

    using Ports = std::unordered_map<fluir::ID, PortSet>;

    Rect moveGrip(const Rect& frame, double unit) {
      return {frame.x + frame.w - (DRAG_SIZE + DRAG_INSET) * unit,
              frame.y + DRAG_INSET * unit,
              DRAG_SIZE * unit,
              DRAG_SIZE * unit};
    }

    Rect resizeBar(const Rect& r, double unit) {
      return {r.x + r.w - RESIZE_BAR_WIDTH * unit, r.y, RESIZE_BAR_WIDTH * unit, r.h};
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

    // A node's body, then its grips over it. `resize` is ResizeX (right bar) or ResizeXY (corner).
    void pushNodeBoxes(const FullID& path,
                       const Rect& rect,
                       const std::optional<Rect>& clip,
                       Part resize,
                       double unit,
                       std::vector<Box>& out) {
      const Rect grip = resize == Part::ResizeXY ? resizeCorner(rect, unit) : resizeBar(rect, unit);
      out.push_back({path, Part::Body, rect, clip});
      out.push_back({path, resize, grip, clip});
      out.push_back({path, Part::MoveGrip, moveGrip(rect, unit), clip});
    }

    // Nodes paint in (z, id) order with their grips; conduits paint after, from ports resolved by id.
    // A container node lays out its own blocks here once one exists.
    void layoutBlock(const pt::Block& block,
                     const FullID& parent,
                     Vec2 origin,
                     const Rect& clip,
                     const EditorContext::Layout& layout,
                     Ports& ports,
                     std::vector<Box>& out) {
      for (const pt::Node* node : sortedNodes(block)) {
        const FullID path = childOf(parent, idOf(*node));
        const Rect rect = atOrigin(origin, localRect(locationOf(*node), layout.unitPx));
        const Part resize = std::holds_alternative<pt::Comment>(*node) ? Part::ResizeXY : Part::ResizeX;
        pushNodeBoxes(path, rect, clip, resize, layout.unitPx, out);
        ports[idOf(*node)] = editor::ports(*node, rect, layout);
      }

      // A dangling endpoint is a legitimate authoring state: it just draws no line.
      for (const pt::Conduit* conduit : sortedConduits(block)) {
        const auto source = ports.find(conduit->input);
        if (source == ports.end() || source->second.outputs.empty()) {
          continue;
        }
        const Vec2 from = source->second.outputs.front();
        for (const pt::Conduit::Output& target : conduit->children) {
          const auto sink = ports.find(target.target);
          if (sink == ports.end() || target.index < 0 ||
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

      Ports ports;
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
          ports[params[row]->id] = draw::anchors(fn, params[row]->id, rail);
        }
      }
      if (fn.output && fn.output->ret) {
        const Rect rail{origin.x + (fn.location.width - layout.returnInsetUnits) * unit,
                        origin.y,
                        layout.railStep(),
                        layout.railStep()};
        out.push_back({childOf(path, fn.output->ret->id), Part::Rail, rail, clip});
        ports[fn.output->ret->id] = draw::anchors(fn, fn.output->ret->id, rail);
      }

      layoutBlock(fn.body, path, origin, clip, layout, ports, out);

      // The frame's chrome paints, and so hits, over its body.
      const Rect header{frame.x, frame.y, frame.w, layout.headerH()};
      out.push_back({path, Part::Frame, frame, std::nullopt});
      out.push_back({path, Part::ResizeXY, resizeCorner(frame, unit), std::nullopt});
      out.push_back({path, Part::MoveGrip, moveGrip(header, unit), std::nullopt});
    }

    bool hittable(Part part) { return part != Part::Frame && part != Part::Rail && part != Part::Wire; }

    // A node's or rail's port anchors, given its box.
    PortSet portsOf(const pt::ParseTree& tree, const Box& box, const EditorContext::Layout& layout) {
      if (box.path.size() < 2) {
        return {};
      }
      if (box.part == Part::Body) {
        const pt::Node* node = nodeAt(tree, box.path);
        return node == nullptr ? PortSet{} : ports(*node, box.world, layout);
      }
      const pt::FunctionDecl* fn = box.part == Part::Rail ? functionAt(tree, parentOf(box.path)) : nullptr;
      return fn == nullptr ? PortSet{} : draw::anchors(*fn, box.path.back(), box.world);
    }

  }  // namespace

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

  std::optional<PortHit> portAt(const pt::ParseTree& tree,
                                std::span<const Box> boxes,
                                Vec2 world,
                                const EditorContext::Layout& layout) {
    const double side = PORT_HIT_UNITS * layout.unitPx;
    for (auto it = boxes.rbegin(); it != boxes.rend(); ++it) {
      if (it->clip && !it->clip->contains(world)) {
        continue;
      }
      const PortSet set = portsOf(tree, *it, layout);
      for (const bool output : {false, true}) {
        const std::vector<Vec2>& anchors = output ? set.outputs : set.inputs;
        for (std::size_t i = 0; i < anchors.size(); ++i) {
          if (dotRect(anchors[i], side).contains(world)) {
            return PortHit{it->path, output, static_cast<int>(i), anchors[i]};
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
