#ifndef FLUIR_EDITOR_TEST_SCENE_PROBE_HPP
#define FLUIR_EDITOR_TEST_SCENE_PROBE_HPP

#include <algorithm>
#include <cstddef>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QPolygonF>
#include <string>
#include <typeinfo>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "compiler/models/id.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/node_access.hpp"
#include "editor/core/tree.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/scene/components/branch_arrow.hpp"
#include "editor/scene/components/choice.hpp"
#include "editor/scene/components/move_grip.hpp"
#include "editor/scene/components/port.hpp"
#include "editor/scene/components/resize_grip.hpp"
#include "editor/scene/components/terminal.hpp"
#include "editor/scene/components/text_edit.hpp"
#include "editor/scene/components/toggle.hpp"
#include "editor/scene/elements/branch_element.hpp"
#include "editor/scene/elements/conduit_element.hpp"
#include "editor/scene/elements/element.hpp"
#include "editor/scene/elements/rail_element.hpp"
#include "editor/view/graph_layout.hpp"

// Reads a GraphScene back as plain values, so tests compare scenes, and scenes against the SDL layout.

namespace testutil {

  using fluir::FullID;
  using fluir::editor::Part;
  using fluir::editor::Rect;
  using fluir::editor::Vec2;

  inline Rect toRect(const QRectF& r) { return {r.x(), r.y(), r.width(), r.height()}; }
  inline Vec2 toVec(const QPointF& p) { return {p.x(), p.y()}; }

  /** `item`'s nearest Element ancestor, or null. */
  inline const fluir::editor::Element* ownerOf(const QGraphicsItem* item) {
    for (const QGraphicsItem* up = item->parentItem(); up != nullptr; up = up->parentItem()) {
      if (const auto* block = dynamic_cast<const fluir::editor::Element*>(up)) {
        return block;
      }
    }
    return nullptr;
  }

  /** `block`'s components of type T: descendants reached without passing through another Element,
   *  skipping the no-contents layer that holds them. */
  template <typename T>
  std::vector<const T*> componentsOf(const QGraphicsItem* block) {
    std::vector<const T*> out;
    // childItems() returns a fresh list, so take it once per item.
    const QList<QGraphicsItem*> roots = block->childItems();
    std::vector<const QGraphicsItem*> todo(roots.begin(), roots.end());
    while (!todo.empty()) {
      const QGraphicsItem* item = todo.back();
      todo.pop_back();
      if (dynamic_cast<const fluir::editor::Element*>(item) != nullptr) {
        continue;
      }
      const bool layer = (item->flags() & QGraphicsItem::ItemHasNoContents) != 0;
      if (const auto* component = dynamic_cast<const T*>(item); component && !layer) {
        out.push_back(component);
      }
      const QList<QGraphicsItem*> children = item->childItems();
      todo.insert(todo.end(), children.begin(), children.end());
    }
    return out;
  }

  /** Every item of type T in `scene`. */
  template <typename T>
  std::vector<const T*> itemsOf(const QGraphicsScene& scene) {
    std::vector<const T*> out;
    for (const QGraphicsItem* item : scene.items()) {
      if (const auto* typed = dynamic_cast<const T*>(item)) {
        out.push_back(typed);
      }
    }
    return out;
  }

  /** Each subpath of `conduit` as its (first, last) point, in scene px. */
  inline std::vector<std::pair<Vec2, Vec2>> wiresOf(const fluir::editor::ConduitElement& conduit) {
    std::vector<std::pair<Vec2, Vec2>> out;
    for (const QPolygonF& poly : conduit.mapToScene(conduit.path()).toSubpathPolygons()) {
      if (!poly.isEmpty()) {
        out.emplace_back(toVec(poly.front()), toVec(poly.back()));
      }
    }
    return out;
  }

  inline std::string str(double v) { return fmt::format("{:.6f}", v + 0.0); }
  inline std::string str(Vec2 v) { return fmt::format("({},{})", str(v.x), str(v.y)); }
  inline std::string str(const Rect& r) { return fmt::format("[{},{} {}x{}]", str(r.x), str(r.y), str(r.w), str(r.h)); }
  inline std::string str(const FullID& path) { return fmt::format("{}", path); }
  inline std::string fieldStr(fluir::editor::Field f) {
    return fmt::format("f{}.{}", static_cast<int>(f.kind), f.index);
  }

  /** The SDL layout's key for `item`, or "" for an item it has no box for. Terminals compare by centre. */
  inline std::string layoutKey(const QGraphicsItem* item) {
    using namespace fluir::editor;
    const Rect r = toRect(item->sceneBoundingRect());
    const auto owner = [item] { return ownerOf(item) ? str(ownerOf(item)->modelPath()) : std::string{"-"}; };
    if (const auto* x = dynamic_cast<const RailElement*>(item))
      return fmt::format("{} Rail {}", str(x->modelPath()), str(r));
    if (const auto* x = dynamic_cast<const BranchElement*>(item)) {
      return fmt::format("{} Branch {}", str(x->modelPath()), str(r));
    }
    if (const auto* x = dynamic_cast<const Element*>(item))
      return fmt::format("{} Body {}", str(x->modelPath()), str(r));
    if (const auto* x = dynamic_cast<const TextEdit*>(item)) {
      return fmt::format("{} Label {} {}", str(x->modelPath()), fieldStr(x->field()), str(r));
    }
    if (const auto* x = dynamic_cast<const Choice*>(item)) {
      return fmt::format("{} Label {} {}", str(x->modelPath()), fieldStr(x->field()), str(r));
    }
    if (const auto* x = dynamic_cast<const Toggle*>(item)) {
      return fmt::format("{} Label {} {}", str(x->modelPath()), fieldStr(Field{Field::Kind::Bool}), str(r));
    }
    if (const auto* x = dynamic_cast<const Terminal*>(item)) {
      return fmt::format(
        "{} Terminal {}{} {}", str(x->modelPath()), x->output() ? "out" : "in", x->index(), str(r.center()));
    }
    if (const auto* x = dynamic_cast<const Port*>(item)) {
      return fmt::format("{} Port {}{} {}", owner(), x->port().output ? "out" : "in", x->port().index, str(r));
    }
    if (dynamic_cast<const MoveGrip*>(item) != nullptr) return fmt::format("{} MoveGrip {}", owner(), str(r));
    if (const auto* x = dynamic_cast<const ResizeGrip*>(item)) {
      return fmt::format("{} Resize{} {}", owner(), static_cast<int>(x->part()), str(r));
    }
    return {};
  }

  /** The same key for an SDL layout box, or "" for a box the scene draws differently (frame, header, wire). */
  inline std::string layoutKey(const fluir::editor::Box& box) {
    using namespace fluir::editor;
    switch (box.part) {
      case Part::Body:
        return fmt::format("{} Body {}", str(box.path), str(box.world));
      case Part::Rail:
        return fmt::format("{} Rail {}", str(box.path), str(box.world));
      case Part::Branch:
        return fmt::format("{} Branch {}", str(box.path), str(box.world));
      case Part::Label:
        return fmt::format("{} Label {} {}", str(box.path), fieldStr(*box.field), str(box.world));
      case Part::Terminal:
        return fmt::format("{} Terminal {}{} {}",
                           str(box.path),
                           box.terminal->output ? "out" : "in",
                           box.terminal->index,
                           str(box.world.center()));
      case Part::Port:
        return fmt::format(
          "{} Port {}{} {}", str(box.path), box.port->output ? "out" : "in", box.port->index, str(box.world));
      case Part::MoveGrip:
        return fmt::format("{} MoveGrip {}", str(box.path), str(box.world));
      case Part::ResizeX:
      case Part::ResizeY:
      case Part::ResizeXY:
        return fmt::format("{} Resize{} {}", str(box.path), static_cast<int>(box.part), str(box.world));
      default:
        return {};
    }
  }

  /** Every item of `scene` in paint order (bottom first), with its kind, geometry and values. Two scenes that paint
   *  the same are equal. */
  inline std::vector<std::string> snapshot(const QGraphicsScene& scene) {
    using namespace fluir::editor;
    std::vector<std::string> out;
    for (const QGraphicsItem* item : scene.items(Qt::AscendingOrder)) {
      std::string line =
        fmt::format("{} {} {}", typeid(*item).name(), layoutKey(item), str(toRect(item->sceneBoundingRect())));
      if (const auto* x = dynamic_cast<const TextEdit*>(item)) line += " '" + x->text().toStdString() + "'";
      if (const auto* x = dynamic_cast<const Choice*>(item)) line += " '" + x->text().toStdString() + "'";
      if (const auto* x = dynamic_cast<const Toggle*>(item)) line += x->value() ? " true" : " false";
      if (const auto* x = dynamic_cast<const ConduitElement*>(item)) {
        line += " " + str(x->modelPath());
        for (const auto& [from, to] : wiresOf(*x))
          line += " " + str(from) + "->" + str(to);
      }
      if (item->flags() & QGraphicsItem::ItemClipsChildrenToShape) line += " clips";
      out.push_back(std::move(line));
    }
    return out;
  }

  /** Every path the scene must hold an item for: declarations, rails, nodes, shown branches and conduits. */
  struct TreePaths {
    std::vector<FullID> blocks;
    std::vector<FullID> conduits;
  };

  inline void collectBlock(const fluir::editor::et::Block& block, const FullID& parent, TreePaths& out) {
    using namespace fluir::editor;
    for (const auto& [id, node] : block.nodes) {
      FullID path = parent;
      path.push_back(id);
      out.blocks.push_back(path);
      if (const auto* conditional = std::get_if<et::Conditional>(&node)) {
        FullID branch = path;
        branch.push_back(conditional->annotation.shownBranch);
        out.blocks.push_back(branch);
        if (const et::Block* shown = branchBlock(*conditional, conditional->annotation.shownBranch)) {
          collectBlock(*shown, branch, out);
        }
      }
    }
    for (const auto& [id, conduit] : block.conduits) {
      FullID path = parent;
      path.push_back(id);
      out.conduits.push_back(path);
    }
  }

  inline TreePaths treePaths(const fluir::editor::et::ParseTree& tree) {
    using namespace fluir::editor;
    TreePaths out;
    for (const auto& [id, decl] : tree.declarations) {
      out.blocks.push_back(FullID{id});
      if (const auto* fn = std::get_if<et::FunctionDecl>(&decl)) {
        for (const auto* param : sortedParameters(*fn))
          out.blocks.push_back(FullID{id, param->id});
        if (fn->output && fn->output->ret) out.blocks.push_back(FullID{id, fn->output->ret->id});
        collectBlock(fn->body, FullID{id}, out);
      }
    }
    return out;
  }

}  // namespace testutil

#endif
