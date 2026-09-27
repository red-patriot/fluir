#include "editor/view/graph_layout.hpp"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <variant>

#include "editor/core/node_access.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/draw_utils.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/graph_geometry.hpp"
#include "editor/view/node_view.hpp"

namespace fluir::editor {
  namespace {
    // A resize edge's thickness, in grid units.
    constexpr double GRIP_THICKNESS = 0.8;
    // A terminal's hit square side, in grid units.
    constexpr double PORT_HIT_UNITS = 1;

    using Terminals = std::unordered_map<fluir::ID, TerminalSet>;

    // A draggable edge is a thick line lying just inside the rect's border; the whole line is the grip.
    Rect resizeEdgeX(const Rect& r, double unit) {
      return {r.x + r.w - GRIP_THICKNESS * unit, r.y, GRIP_THICKNESS * unit, r.h};
    }

    FullID childOf(const FullID& parent, fluir::ID id) {
      FullID out = parent;
      out.push_back(id);
      return out;
    }

    void pushLabels(const FullID& path,
                    const std::vector<FieldLabel>& labels,
                    const std::optional<Rect>& clip,
                    std::vector<Box>& out) {
      for (const FieldLabel& label : labels) {
        out.push_back({path, Part::Label, label.rect, clip, label.field});
      }
    }

    // One Terminal box per anchor in `set`, so each dot paints and hits at `path`.
    void pushTerminals(
      const FullID& path, const TerminalSet& set, const std::optional<Rect>& clip, double unit, std::vector<Box>& out) {
      for (const bool output : {false, true}) {
        const std::vector<Vec2>& anchors = output ? set.outputs : set.inputs;
        for (std::size_t i = 0; i < anchors.size(); ++i) {
          out.push_back({.path = path,
                         .part = Part::Terminal,
                         .world = dotRect(anchors[i], PORT_HIT_UNITS * unit),
                         .clip = clip,
                         .terminal = BoxTerminal{output, i}});
        }
      }
    }

    // A node's body and labels, then its grips over them. `resize` is ResizeX (right edge), ResizeXY (corner) or none.
    void pushNodeBoxes(const FullID& path,
                       const Rect& rect,
                       const std::optional<Rect>& clip,
                       std::optional<Part> resize,
                       const std::vector<FieldLabel>& labels,
                       double unit,
                       std::vector<Box>& out) {
      out.push_back({path, Part::Body, rect, clip});
      pushLabels(path, labels, clip, out);
      if (resize) {
        const Rect grip = *resize == Part::ResizeXY ? resizeCorner(rect, unit) : resizeEdgeX(rect, unit);
        out.push_back({path, *resize, grip, clip});
      }
      out.push_back({path, Part::MoveGrip, moveGrip(rect, unit), clip});
    }

    void layoutBlock(const et::Block& block,
                     const FullID& parent,
                     Vec2 origin,
                     const Rect& clip,
                     const EditorContext::Layout& layout,
                     Terminals terminals,
                     std::vector<Box>& out);

    // The conditional's body, its visible branch's nodes, then its chrome over them.
    void layoutConditional(const et::Conditional& conditional,
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
      const FullID branchPath = childOf(path, conditional.annotation.shownBranch);
      const et::Block* branch = branchBlock(conditional, conditional.annotation.shownBranch);
      const Terminals inner = draw::innerAnchors(conditional, frame, layout);
      const std::optional<Rect> contentClip = clip ? intersect(*clip, content) : std::optional<Rect>{content};
      if (contentClip) {
        out.push_back({branchPath, Part::Branch, content, contentClip});
        if (branch) {
          layoutBlock(*branch, branchPath, origin, *contentClip, layout, inner, out);
        }
      }

      // The frame's chrome paints, and so hits, over its branch; port terminals paint over their port.
      const Rect header{frame.x, frame.y, frame.w, layout.headerH()};
      out.push_back({path, Part::Frame, frame, clip});
      out.push_back({.path = path,
                     .part = Part::Port,
                     .world = draw::conditionPortRect(conditional, frame, layout),
                     .clip = clip,
                     .port = PortRef{.output = false, .index = 0}});
      // TODO: layoutBlock already computed these anchors for wiring; pass them in instead of recomputing.
      pushTerminals(path, draw::anchors(conditional, frame, layout), clip, unit, out);
      if (contentClip) {
        for (const auto& [innerId, set] : inner) {
          pushTerminals(childOf(branchPath, innerId), set, contentClip, unit, out);
        }
      }
      out.push_back({path, Part::Header, header, clip});
      if (const std::optional<Part> resize = draw::resizePart(conditional)) {
        out.push_back({path, *resize, resizeCorner(frame, unit), clip});
      }
      out.push_back({path, Part::MoveGrip, moveGrip(header, unit), clip});
    }

    // Nodes paint in (z, id) order with their grips; conduits paint after, from terminals resolved by id.
    // `terminals` is per block: node ids repeat across sibling blocks.
    void layoutBlock(const et::Block& block,
                     const FullID& parent,
                     Vec2 origin,
                     const Rect& clip,
                     const EditorContext::Layout& layout,
                     Terminals terminals,
                     std::vector<Box>& out) {
      for (const et::Node* node : sortedNodes(block)) {
        const FullID path = childOf(parent, idOf(*node));
        const Rect rect = atOrigin(origin, localRect(locationOf(*node), layout.unitPx));
        const TerminalSet set = editor::terminals(*node, rect, layout);
        if (const auto* conditional = std::get_if<et::Conditional>(node)) {
          layoutConditional(*conditional, path, rect, clip, layout, out);
        } else {
          pushNodeBoxes(path, rect, clip, nodeResizePart(*node), nodeLabels(*node, rect, layout), layout.unitPx, out);
          pushTerminals(path, set, clip, layout.unitPx, out);
        }
        terminals[idOf(*node)] = set;
      }

      // A dangling endpoint is a legitimate authoring state: it just draws no line.
      for (const et::Conduit* conduit : sortedConduits(block)) {
        const auto source = terminals.find(conduit->input);
        if (source == terminals.end() || source->second.outputs.empty()) {
          continue;
        }
        const Vec2 from = source->second.outputs.front();
        for (const et::Conduit::Output& target : conduit->children) {
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

    void layoutFunction(const et::FunctionDecl& fn, const EditorContext::Layout& layout, std::vector<Box>& out) {
      const FullID path{fn.id};
      const double unit = layout.unitPx;
      const Rect frame = localRect(fn.location, unit);
      const Vec2 origin = bodyOrigin(frame.topLeft(), layout.headerH());
      const Rect clip{frame.x, origin.y, frame.w, frame.h - layout.headerH()};
      out.push_back({path, Part::Body, frame, std::nullopt});

      Terminals terminals;
      if (fn.input) {
        const std::vector<const et::FunctionDecl::Parameter*> params = sortedParameters(fn);
        for (std::size_t row = 0; row < params.size(); ++row) {
          const Rect rail{
            origin.x, origin.y + static_cast<double>(row) * layout.railStep(), layout.paramW(), layout.railStep()};
          const FullID railPath = childOf(path, params[row]->id);
          out.push_back({railPath, Part::Rail, rail, clip});
          pushLabels(path, draw::labels(fn, params[row]->id, rail, layout), clip, out);
          terminals[params[row]->id] = draw::anchors(fn, params[row]->id, rail);
          pushTerminals(railPath, terminals[params[row]->id], clip, unit, out);
        }
      }
      if (fn.output && fn.output->ret) {
        const Rect rail{origin.x + (fn.location.width - layout.returnInsetUnits) * unit,
                        origin.y,
                        layout.railStep(),
                        layout.railStep()};
        const FullID railPath = childOf(path, fn.output->ret->id);
        out.push_back({railPath, Part::Rail, rail, clip});
        pushLabels(path, draw::labels(fn, fn.output->ret->id, rail, layout), clip, out);
        terminals[fn.output->ret->id] = draw::anchors(fn, fn.output->ret->id, rail);
        pushTerminals(railPath, terminals[fn.output->ret->id], clip, unit, out);
      }

      layoutBlock(fn.body, path, origin, clip, layout, std::move(terminals), out);

      // The frame's chrome paints, and so hits, over its body.
      const Rect header{frame.x, frame.y, frame.w, layout.headerH()};
      out.push_back({path, Part::Frame, frame, std::nullopt});
      out.push_back({path, Part::Header, header, std::nullopt});
      pushLabels(path, draw::labels(fn, frame, layout), std::nullopt, out);
      out.push_back({path, Part::ResizeXY, resizeCorner(frame, unit), std::nullopt});
      out.push_back({path, Part::MoveGrip, moveGrip(header, unit), std::nullopt});
    }

    bool hittable(Part part) { return part != Part::Frame && part != Part::Wire && part != Part::Terminal; }

    // The last-painted hittable box containing `world`, Labels included unless `throughLabels`.
    const Box* topAt(std::span<const Box> boxes, Vec2 world, bool throughLabels) {
      for (auto it = boxes.rbegin(); it != boxes.rend(); ++it) {
        if (hittable(it->part) && !(throughLabels && it->part == Part::Label) &&
            (!it->clip || it->clip->contains(world)) && it->world.contains(world)) {
          return &*it;
        }
      }
      return nullptr;
    }

  }  // namespace

  std::vector<Box> layoutGraph(const et::ParseTree& tree, const EditorContext::Layout& layout) {
    std::vector<Box> out;
    for (const et::Declaration* decl : sortedDeclarations(tree)) {
      if (const auto* fn = std::get_if<et::FunctionDecl>(decl)) {
        layoutFunction(*fn, layout, out);
      } else if (const auto* comment = std::get_if<et::Comment>(decl)) {
        const Rect rect = localRect(comment->location, layout.unitPx);
        pushNodeBoxes(FullID{comment->id},
                      rect,
                      std::nullopt,
                      draw::resizePart(*comment),
                      draw::labels(*comment, rect, layout),
                      layout.unitPx,
                      out);
      }
    }
    return out;
  }

  const Box* hitAt(std::span<const Box> boxes, Vec2 world) { return topAt(boxes, world, true); }

  const Box* labelAt(std::span<const Box> boxes, Vec2 world) {
    const Box* top = topAt(boxes, world, false);
    return top != nullptr && top->part == Part::Label ? top : nullptr;
  }

  std::optional<TerminalHit> terminalAt(std::span<const Box> boxes, Vec2 world) {
    for (auto it = boxes.rbegin(); it != boxes.rend(); ++it) {
      if (it->terminal && (!it->clip || it->clip->contains(world)) && it->world.contains(world)) {
        const Vec2 anchor{it->world.x + it->world.w / 2, it->world.y + it->world.h / 2};
        return TerminalHit{it->path, it->terminal->output, static_cast<int>(it->terminal->index), anchor};
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
