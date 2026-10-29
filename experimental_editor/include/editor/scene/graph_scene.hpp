#ifndef FLUIR_EDITOR_SCENE_GRAPH_SCENE_HPP
#define FLUIR_EDITOR_SCENE_GRAPH_SCENE_HPP

#include <map>
#include <QGraphicsScene>
#include <unordered_map>

#include "compiler/models/id.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/core/geometry.hpp"
#include "editor/core/tree.hpp"
#include "editor/scene/elements/conduit_element.hpp"
#include "editor/scene/elements/element.hpp"
#include "editor/view/draw/draw_utils.hpp"

namespace fluir::editor {

  /** A keyed projection of a parse tree: one item per declaration, node, shown branch, rail and conduit. */
  class GraphScene : public QGraphicsScene {
   public:
    explicit GraphScene(const EditorContext& ctx, QObject* parent = nullptr);
    ~GraphScene() override;

    /** Brings the items in line with `tree`: keeps an item whose path and kind survive, recreates one whose kind
     *  changed, syncs every field, and removes items whose paths are gone. */
    void reconcile(const et::ParseTree& tree);

    /** The item at `path`, or null. */
    Element* itemAt(const FullID& path) const;
    /** The conduit at `path`, or null. */
    ConduitElement* conduitAt(const FullID& path) const;

   private:
    struct Pass;

    template <typename Item>
    Item* ensure(Pass& pass, const FullID& path, QGraphicsItem* parent);
    ConduitElement* ensureConduit(Pass& pass, const FullID& path, QGraphicsItem* parent);
    void syncFunction(Pass& pass, const et::ParseTree& tree, const et::FunctionDecl& fn, double z);
    void syncBlock(Pass& pass,
                   const et::ParseTree& tree,
                   const et::Block& block,
                   const FullID& parent,
                   QGraphicsItem* container,
                   Vec2 origin,
                   std::unordered_map<fluir::ID, TerminalSet> terminals,
                   double z);
    /** Deletes `item` and forgets every path it and its descendants held. */
    void destroy(QGraphicsItem* item);

    const EditorContext& ctx_;
    // Non-owning; the Qt item tree owns. Ordered, since FullID (a vector) has no std::hash.
    std::map<FullID, Element*> items_;
    std::map<FullID, ConduitElement*> conduits_;
  };

}  // namespace fluir::editor

#endif
