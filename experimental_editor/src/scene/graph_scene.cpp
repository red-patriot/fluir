#include "editor/scene/graph_scene.hpp"

#include <cstddef>
#include <set>
#include <utility>
#include <variant>
#include <vector>

#include "editor/core/node_access.hpp"
#include "editor/core/tree_path.hpp"
#include "editor/scene/components/scope_region.hpp"
#include "editor/scene/elements/binary_element.hpp"
#include "editor/scene/elements/branch_element.hpp"
#include "editor/scene/elements/call_element.hpp"
#include "editor/scene/elements/comment_element.hpp"
#include "editor/scene/elements/conditional_element.hpp"
#include "editor/scene/elements/conduit_element.hpp"
#include "editor/scene/elements/constant_element.hpp"
#include "editor/scene/elements/element.hpp"
#include "editor/scene/elements/function_element.hpp"
#include "editor/scene/elements/rail_element.hpp"
#include "editor/scene/elements/unary_element.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/graph_geometry.hpp"
#include "editor/view/node_view.hpp"

namespace fluir::editor {
  namespace {

    FullID childOf(const FullID& parent, fluir::ID id) {
      FullID out = parent;
      out.push_back(id);
      return out;
    }

  }  // namespace

  /** The paths one reconcile reached; everything else goes. */
  struct GraphScene::Pass {
    std::set<FullID> items;
    std::set<FullID> conduits;
  };

  GraphScene::GraphScene(const EditorContext& ctx, QObject* parent) : QGraphicsScene(parent), ctx_(ctx) { }

  GraphScene::~GraphScene() = default;

  void GraphScene::reconcile(const et::ParseTree& tree) {
    Pass pass;
    double z = 0;
    for (const et::Declaration* decl : sortedDeclarations(tree)) {
      if (const auto* fn = std::get_if<et::FunctionDecl>(decl)) {
        syncFunction(pass, tree, *fn, z++);
      } else if (const auto* comment = std::get_if<et::Comment>(decl)) {
        auto* item = ensure<CommentElement>(pass, FullID{comment->id}, nullptr);
        item->setZValue(z++);
        item->sync(tree, *comment, localRect(comment->location, ctx_.layout.unitPx));
      }
    }

    // A parent goes before its children, which takes them out of the maps with it.
    std::vector<QGraphicsItem*> gone;
    for (const auto& [path, item] : items_) {
      if (!pass.items.contains(path)) gone.push_back(item);
    }
    for (const auto& [path, item] : conduits_) {
      if (!pass.conduits.contains(path)) gone.push_back(item);
    }
    for (QGraphicsItem* item : gone) {
      auto* block = dynamic_cast<Element*>(item);
      auto* conduit = dynamic_cast<ConduitElement*>(item);
      if ((block && itemAt(block->modelPath()) == block) || (conduit && conduitAt(conduit->modelPath()) == conduit)) {
        destroy(item);
      }
    }
  }

  Element* GraphScene::itemAt(const FullID& path) const {
    const auto it = items_.find(path);
    return it == items_.end() ? nullptr : it->second;
  }

  ConduitElement* GraphScene::conduitAt(const FullID& path) const {
    const auto it = conduits_.find(path);
    return it == conduits_.end() ? nullptr : it->second;
  }

  template <typename Item>
  Item* GraphScene::ensure(Pass& pass, const FullID& path, QGraphicsItem* parent) {
    pass.items.insert(path);
    if (const auto it = items_.find(path); it != items_.end()) {
      if (auto* item = dynamic_cast<Item*>(it->second)) {
        if (item->parentItem() != parent) item->setParentItem(parent);
        return item;
      }
      destroy(it->second);  // its kind changed
    }
    auto* item = new Item(path, ctx_, parent);
    if (parent == nullptr) addItem(item);
    items_[path] = item;
    return item;
  }

  ConduitElement* GraphScene::ensureConduit(Pass& pass, const FullID& path, QGraphicsItem* parent) {
    pass.conduits.insert(path);
    if (const auto it = conduits_.find(path); it != conduits_.end()) {
      if (it->second->parentItem() != parent) it->second->setParentItem(parent);
      return it->second;
    }
    auto* item = new ConduitElement(path, ctx_, parent);
    conduits_[path] = item;
    return item;
  }

  void GraphScene::syncFunction(Pass& pass, const et::ParseTree& tree, const et::FunctionDecl& fn, double z) {
    const EditorContext::Layout& layout = ctx_.layout;
    const FullID path{fn.id};
    auto* item = ensure<FunctionElement>(pass, path, nullptr);
    item->setZValue(z);
    item->sync(tree, fn);

    // Rails paint under the body's nodes, in their layout order.
    const Rect frame = localRect(fn.location, layout.unitPx);
    std::vector<fluir::ID> rails;
    if (fn.input) {
      for (const auto* param : sortedParameters(fn))
        rails.push_back(param->id);
    }
    if (fn.output && fn.output->ret) rails.push_back(fn.output->ret->id);

    std::unordered_map<fluir::ID, TerminalSet> terminals;
    double railZ = 0;
    for (const fluir::ID id : rails) {
      const Rect rail = draw::railRect(fn, id, frame, layout);
      auto* railItem = ensure<RailElement>(pass, childOf(path, id), item->scope());
      railItem->setZValue(railZ++);
      railItem->sync(tree, fn, id, rail);
      terminals[id] = draw::anchors(fn, id, rail);
    }
    syncBlock(pass,
              tree,
              fn.body,
              path,
              item->scope(),
              bodyOrigin(frame.topLeft(), layout.headerH()),
              std::move(terminals),
              railZ);
  }

  // Nodes stack in (z, id) order and conduits over them; `terminals` is per block, since ids repeat across blocks.
  void GraphScene::syncBlock(Pass& pass,
                             const et::ParseTree& tree,
                             const et::Block& block,
                             const FullID& parent,
                             QGraphicsItem* container,
                             Vec2 origin,
                             std::unordered_map<fluir::ID, TerminalSet> terminals,
                             double z) {
    const EditorContext::Layout& layout = ctx_.layout;
    for (const et::Node* node : sortedNodes(block)) {
      const FullID path = childOf(parent, idOf(*node));
      const Rect rect = atOrigin(origin, localRect(locationOf(*node), layout.unitPx));
      std::visit(
        [&]<typename N>(const N& n) {
          if constexpr (std::is_same_v<N, et::Conditional>) {
            auto* item = ensure<ConditionalElement>(pass, path, container);
            item->setZValue(z);
            item->sync(tree, n, rect);
            const FullID branchPath = childOf(path, n.annotation.shownBranch);
            auto* branch = ensure<BranchElement>(pass, branchPath, item);
            branch->setZValue(0);
            branch->sync(n, rect, contentRect(rect, layout.headerH()));
            if (const et::Block* shown = branchBlock(n, n.annotation.shownBranch)) {
              syncBlock(pass,
                        tree,
                        *shown,
                        branchPath,
                        branch,
                        bodyOrigin(rect.topLeft(), layout.headerH()),
                        draw::innerAnchors(n, rect, layout),
                        0);
            }
          } else {
            using Item = std::conditional_t<
              std::is_same_v<N, et::Constant>,
              ConstantElement,
              std::conditional_t<
                std::is_same_v<N, et::Binary>,
                BinaryElement,
                std::conditional_t<std::is_same_v<N, et::Unary>,
                                   UnaryElement,
                                   std::conditional_t<std::is_same_v<N, et::Call>, CallElement, CommentElement>>>>;
            auto* item = ensure<Item>(pass, path, container);
            item->setZValue(z);
            item->sync(tree, n, rect);
          }
        },
        *node);
      ++z;
      terminals[idOf(*node)] = editor::terminals(*node, rect, layout);
    }

    // A dangling endpoint is a legitimate authoring state: it just draws no line.
    for (const et::Conduit* conduit : sortedConduits(block)) {
      std::vector<std::pair<Vec2, Vec2>> wires;
      const auto source = terminals.find(conduit->input);
      if (source != terminals.end() && conduit->index >= 0 &&
          static_cast<std::size_t>(conduit->index) < source->second.outputs.size()) {
        const Vec2 from = source->second.outputs[static_cast<std::size_t>(conduit->index)];
        for (const et::Conduit::Output& target : conduit->children) {
          const auto sink = terminals.find(target.target);
          if (sink != terminals.end() && target.index >= 0 &&
              static_cast<std::size_t>(target.index) < sink->second.inputs.size()) {
            wires.emplace_back(from, sink->second.inputs[static_cast<std::size_t>(target.index)]);
          }
        }
      }
      auto* item = ensureConduit(pass, childOf(parent, conduit->id), container);
      item->setZValue(z++);
      item->sync(wires);
    }
  }

  void GraphScene::destroy(QGraphicsItem* item) {
    std::vector<QGraphicsItem*> todo{item};
    while (!todo.empty()) {
      QGraphicsItem* next = todo.back();
      todo.pop_back();
      if (auto* block = dynamic_cast<Element*>(next); block && itemAt(block->modelPath()) == block) {
        items_.erase(block->modelPath());
      }
      if (auto* conduit = dynamic_cast<ConduitElement*>(next); conduit && conduitAt(conduit->modelPath()) == conduit) {
        conduits_.erase(conduit->modelPath());
      }
      const QList<QGraphicsItem*> children = next->childItems();
      todo.insert(todo.end(), children.begin(), children.end());
    }
    delete item;
  }

}  // namespace fluir::editor
