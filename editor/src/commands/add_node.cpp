#include "editor/commands/add_node.hpp"

#include <variant>

#include "editor/diagram/build.hpp"
#include "editor/diagram/ids.hpp"
#include "editor/diagram/node.hpp"
#include "editor/view/draw/binary.hpp"
#include "editor/view/draw/call.hpp"
#include "editor/view/draw/comment.hpp"
#include "editor/view/draw/conditional.hpp"
#include "editor/view/draw/constant.hpp"
#include "editor/view/draw/function.hpp"
#include "editor/view/draw/unary.hpp"
#include "fluir/util/overloaded.hpp"

namespace fluir::editor {
  namespace {

    Vec2i defaultSize(const pt::Node& node) {
      return std::visit(
        util::Overloaded{
          [](const pt::Conditional& n) {
            return draw::defaultSize(draw::Ports{.condition = n.condition, .inputs = n.inputs, .outputs = n.outputs});
          },
          [](const auto& n) { return draw::defaultSize(n); },
        },
        node);
    }

    Vec2i defaultSize(const pt::Declaration& decl) {
      return std::visit(
        util::Overloaded{
          [](const pt::FunctionDecl& fn) {
            return draw::defaultSize(draw::Signature{.name = fn.name, .input = fn.input, .output = fn.output});
          },
          [](const pt::Comment& c) { return draw::defaultSize(c); },
        },
        decl);
    }

    // `prototype` with `id`, at `at` on layer `z`, at its kind's default size.
    template <typename Variant>
    Variant placed(Variant prototype, fluir::ID id, Vec2i at, int z) {
      const Vec2i size = defaultSize(prototype);
      std::visit(
        [&](auto& n) {
          n.id = id;
          n.location = {.x = at.x, .y = at.y, .z = z, .width = size.x, .height = size.y};
        },
        prototype);
      return prototype;
    }

    diagram::FluirNode& built(diagram::Scope& scope, const pt::Node& prototype, Vec2i at) {
      const auto& container = static_cast<const diagram::FluirNode&>(*scope.parentItem());
      return diagram::buildNode(placed(prototype, diagram::nextId(scope), at, container.location().z + 1), scope);
    }

    diagram::FluirNode& built(ModuleScene& scene, const pt::Declaration& prototype, Vec2i at) {
      return diagram::buildDeclaration(placed(prototype, diagram::nextId(scene), at, 0), scene);
    }

  }  // namespace

  AddNode::AddNode(diagram::Scope& scope, const pt::Node& prototype, Vec2i at) : removed_(built(scope, prototype, at)) {
    removed_.remove();
  }

  AddNode::AddNode(ModuleScene& scene, const pt::Declaration& prototype, Vec2i at) :
    removed_(built(scene, prototype, at)) {
    removed_.remove();
  }

  void AddNode::redo() { removed_.restore(); }

  void AddNode::undo() { removed_.remove(); }

}  // namespace fluir::editor
