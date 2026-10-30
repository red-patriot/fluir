#include "editor/diagram/build.hpp"

#include <algorithm>
#include <cstddef>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "editor/diagram/binary.hpp"
#include "editor/diagram/call.hpp"
#include "editor/diagram/comment.hpp"
#include "editor/diagram/conditional.hpp"
#include "editor/diagram/conduit.hpp"
#include "editor/diagram/constant.hpp"
#include "editor/diagram/function.hpp"
#include "editor/diagram/parameter.hpp"
#include "editor/diagram/return.hpp"
#include "editor/diagram/terminal.hpp"
#include "editor/diagram/unary.hpp"
#include "fluir/util/overloaded.hpp"

namespace fluir::editor::diagram {
  namespace {

    class Endpoints {
     public:
      /** Returns true when the claim got a new id, false when it is already taken in this scope. */
      bool claim(fluir::ID id, Terminals terminals = {}) { return map_.try_emplace(id, std::move(terminals)).second; }

      Terminal* source(fluir::ID id, int index) const { return find(id, index, &Terminals::outputs); }
      Terminal* sink(fluir::ID id, int index) const { return find(id, index, &Terminals::inputs); }

     private:
      Terminal* find(fluir::ID id, int index, std::vector<Terminal*> Terminals::*side) const {
        const auto it = map_.find(id);
        if (it == map_.end() || index < 0) {
          return nullptr;
        }
        const std::vector<Terminal*>& terminals = it->second.*side;
        return static_cast<std::size_t>(index) < terminals.size() ? terminals[static_cast<std::size_t>(index)] :
                                                                    nullptr;
      }

      std::unordered_map<fluir::ID, Terminals> map_;
    };

    void collect(Terminals& into, Terminal* terminal) {
      std::vector<Terminal*>& side = terminal->isOutput() ? into.outputs : into.inputs;
      const auto index = static_cast<std::size_t>(terminal->index());
      if (side.size() <= index) {
        side.resize(index + 1, nullptr);
      }
      side[index] = terminal;
    }

    Terminals terminalsOf(const QGraphicsItem& owner) {
      Terminals out;
      for (QGraphicsItem* child : owner.childItems()) {
        if (auto* terminal = dynamic_cast<Terminal*>(child)) {
          collect(out, terminal);
        }
      }
      return out;
    }

    template <typename Variant>
    std::vector<const Variant*> stackOrder(const std::unordered_map<fluir::ID, Variant>& items) {
      std::vector<const Variant*> out;
      for (const auto& entry : items) {
        out.push_back(&entry.second);
      }
      std::ranges::sort(out, {}, [](const Variant* item) {
        return std::visit([](const auto& n) { return std::pair{n.location.z, n.id}; }, *item);
      });
      return out;
    }

    class Builder {
     public:
      explicit Builder(const EditorContext& ctx) : ctx_(ctx) { }

      bool function(const pt::FunctionDecl& fn, ModuleScene& scene) {
        auto* item = new Function(fn.id, fn.location, {.name = fn.name, .input = fn.input, .output = fn.output}, ctx_);
        scene.addItem(item);
        Endpoints endpoints;
        for (QGraphicsItem* child : item->childItems()) {
          const auto* param = dynamic_cast<Parameter*>(child);
          const auto* ret = dynamic_cast<Return*>(child);
          if ((param && !endpoints.claim(param->id(), terminalsOf(*param))) ||
              (ret && !endpoints.claim(ret->id(), terminalsOf(*ret)))) {
            return false;
          }
        }
        return block(fn.body, std::move(endpoints), *item->body());
      }

     private:
      bool block(const pt::Block& block, Endpoints endpoints, QGraphicsItem& scope) {
        for (const pt::Node* node : stackOrder(block.nodes)) {
          if (!this->node(*node, endpoints, scope)) {
            return false;
          }
        }
        std::vector<const pt::Conduit*> conduits;
        for (const auto& entry : block.conduits) {
          conduits.push_back(&entry.second);
        }
        std::ranges::sort(conduits, {}, &pt::Conduit::id);
        for (const pt::Conduit* conduit : conduits) {
          if (!this->conduit(*conduit, endpoints, scope)) {
            return false;
          }
        }
        return true;
      }

      bool node(const pt::Node& node, Endpoints& endpoints, QGraphicsItem& scope) {
        const auto leaf = [&](FluirNode* item) { return endpoints.claim(item->id(), terminalsOf(*item)); };
        return std::visit(fluir::util::Overloaded{
                            [&](const pt::Constant& n) { return leaf(new Constant(n, ctx_, &scope)); },
                            [&](const pt::Binary& n) { return leaf(new Binary(n, ctx_, &scope)); },
                            [&](const pt::Unary& n) { return leaf(new Unary(n, ctx_, &scope)); },
                            [&](const pt::Call& n) { return leaf(new Call(n, ctx_, &scope)); },
                            [&](const pt::Comment& n) { return leaf(new Comment(n, ctx_, &scope)); },
                            [&](const pt::Conditional& n) { return conditional(n, endpoints, scope); },
                          },
                          node);
      }

      bool conditional(const pt::Conditional& node, Endpoints& endpoints, QGraphicsItem& scope) {
        auto* item = new Conditional(node.id,
                                     node.location,
                                     {.condition = node.condition, .inputs = node.inputs, .outputs = node.outputs},
                                     ctx_,
                                     &scope);
        Terminals outer;
        Endpoints inner;
        for (const Port* port : item->wallPorts()) {
          collect(outer, port->outer());
          if (port->inner() != nullptr) {
            Terminals terminals;
            collect(terminals, port->inner());
            if (!inner.claim(port->innerId(), std::move(terminals))) {
              return false;
            }
          }
        }
        return endpoints.claim(node.id, std::move(outer)) && block(*node.thenScope, inner, *item->thenBranch()) &&
               block(*node.elseScope, inner, *item->elseBranch());
      }

      bool conduit(const pt::Conduit& conduit, Endpoints& endpoints, QGraphicsItem& scope) {
        Terminal* source = endpoints.source(conduit.input, conduit.index);
        if (source == nullptr || conduit.children.empty() || !endpoints.claim(conduit.id)) {
          return false;
        }
        std::vector<Terminal*> sinks;
        for (const pt::Conduit::Output& output : conduit.children) {
          sinks.push_back(endpoints.sink(output.target, output.index));
          if (sinks.back() == nullptr) {
            return false;
          }
        }
        new Conduit(conduit.id, source, std::move(sinks), ctx_, &scope);
        return true;
      }

      const EditorContext& ctx_;
    };

  }  // namespace

  bool buildScene(const pt::ParseTree& tree, ModuleScene& scene) {
    scene.clear();
    scene.setHeader(tree.header);
    Builder builder(scene.context());
    for (const pt::Declaration* decl : stackOrder(tree.declarations)) {
      const bool built = std::visit(fluir::util::Overloaded{
                                      [&](const pt::FunctionDecl& fn) { return builder.function(fn, scene); },
                                      [&](const pt::Comment& c) {
                                        scene.addItem(new Comment(c, scene.context(), nullptr));
                                        return true;
                                      },
                                    },
                                    *decl);
      if (!built) {
        scene.clear();
        return false;
      }
    }
    return true;
  }

}  // namespace fluir::editor::diagram
