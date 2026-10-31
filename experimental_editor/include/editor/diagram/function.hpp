#ifndef FLUIR_EDITOR_DIAGRAM_FUNCTION_HPP
#define FLUIR_EDITOR_DIAGRAM_FUNCTION_HPP

#include <cstddef>
#include <memory>
#include <vector>

#include "compiler/models/id.hpp"
#include "compiler/models/location.hpp"
#include "editor/core/editor_context.hpp"
#include "editor/diagram/node.hpp"
#include "editor/diagram/parameter.hpp"
#include "editor/diagram/return.hpp"
#include "editor/diagram/scope.hpp"
#include "editor/view/draw/function.hpp"

namespace fluir::editor::diagram {

  /** Display element for a function declaration. */
  class Function : public FluirNode {
   public:
    /** Data about a removed parameter. */
    struct RemovedParameter {
      std::unique_ptr<Parameter> item;
      pt::FunctionParameter data;
      std::size_t position;
    };

    /** Data about a removed return. */
    struct RemovedReturn {
      std::unique_ptr<Return> item;
      pt::FunctionReturn data;
    };

    Function(fluir::ID id, FlowGraphLocation location, draw::Signature signature, const EditorContext& ctx);

    fluir::ID id() const override { return id_; }
    const FlowGraphLocation& location() const override { return location_; }
    Limits<Vec2i> sizeLimits() const override { return draw::sizeLimits(signature_); }
    std::string text(Field field) const override;
    bool accepts(Field field, std::string_view text) const override;
    std::vector<std::string> choices(Field field) const override;
    const draw::Signature& signature() const { return signature_; }
    /** Where the body's nodes and conduits live. */
    Scope* body() const { return body_; }
    const std::vector<Parameter*>& parameters() const { return parameters_; }
    /** The function's return or null. */
    Return* ret() const { return return_; }

    /** Appends parameter `data`. */
    void addParameter(pt::FunctionParameter data);
    /** Removes parameter `id`. */
    RemovedParameter removeParameter(fluir::ID id);
    /** Puts `removed` back. */
    void restoreParameter(RemovedParameter removed);
    /** Drops the input block if it has no parameters. */
    void dropEmptyInput();

    /** Adds return `data`. */
    void addReturn(pt::FunctionReturn data);
    /** Removes the return. */
    RemovedReturn removeReturn();
    void restoreReturn(RemovedReturn removed);
    /** Drops the output block if it has no return. */
    void dropEmptyOutput();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

   private:
    void storeLocation(const FlowGraphLocation& location) override { location_ = location; }
    void arrange() override;
    void write(Field field, std::string_view text) override;

    fluir::ID id_;
    FlowGraphLocation location_;
    draw::Signature signature_;
    Scope* body_;
    std::vector<Parameter*> parameters_;
    Return* return_ = nullptr;
  };

}  // namespace fluir::editor::diagram

#endif
