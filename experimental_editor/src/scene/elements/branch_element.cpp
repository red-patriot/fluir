#include "editor/scene/elements/branch_element.hpp"

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>

#include "editor/view/draw/conditional.hpp"

namespace fluir::editor {

  BranchElement::BranchElement(FullID path, const EditorContext& ctx, QGraphicsItem* parent) :
    Element(std::move(path), ctx, parent) {
    setFlag(ItemClipsChildrenToShape);
    setFlag(ItemHasNoContents);
  }

  void BranchElement::sync(const et::Conditional& node, const Rect& frame, const Rect& content) {
    place(content);
    clearComponents();
    const std::unordered_map<fluir::ID, TerminalSet> inner = draw::innerAnchors(node, frame, ctx_.layout);
    std::vector<fluir::ID> ids;
    for (const auto& [id, set] : inner) {
      ids.push_back(id);
    }
    std::ranges::sort(ids);
    for (const fluir::ID id : ids) {
      FullID endpoint = modelPath();
      endpoint.push_back(id);
      addTerminals(endpoint, inner.at(id));
    }
  }

  void BranchElement::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) { }

}  // namespace fluir::editor
