#ifndef FLUIR_EDITOR_DIAGRAM_UNARY_HPP
#define FLUIR_EDITOR_DIAGRAM_UNARY_HPP

#include "compiler/frontend/parse_tree/parse_tree.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/diagram/node.hpp"
#include "editor/diagram/terminal.hpp"

namespace fluir::editor::diagram {

  /** Display element for a unary operator. */
  class Unary : public FluirNode {
   public:
    Unary(pt::Unary data, const EditorContext& ctx, QGraphicsItem* parent);

    const pt::Unary& data() const { return data_; }
    fluir::ID id() const override { return data_.id; }
    const FlowGraphLocation& location() const override { return data_.location; }
    Limits<Vec2i> sizeLimits() const override;
    std::string text(Field field) const override;
    bool accepts(Field field, std::string_view text) const override;
    std::vector<std::string> choices(Field field) const override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    void storeLocation(const FlowGraphLocation& location) override { data_.location = location; }
    void arrange() override;
    void write(Field field, std::string_view text) override;

    pt::Unary data_;
    Terminals terminals_;
  };

}  // namespace fluir::editor::diagram

#endif
