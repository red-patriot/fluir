#include "editor/core/annotate.hpp"

#include <utility>
#include <variant>

namespace fluir::editor {
  namespace {

    // One overload per struct that differs between the two instantiations; everything else assigns straight across,
    // because it is the same type on both sides. Annotations all stay at their defaults.

    et::Block convert(const pt::Block& src);
    et::Node convert(const pt::Node& src);

    et::Comment convert(const pt::Comment& src) {
      return et::Comment{.id = src.id, .location = src.location, .text = src.text};
    }

    et::Constant convert(const pt::Constant& src) {
      return et::Constant{.id = src.id, .location = src.location, .value = src.value};
    }

    et::Binary convert(const pt::Binary& src) {
      return et::Binary{.id = src.id, .location = src.location, .lhs = src.lhs, .rhs = src.rhs, .op = src.op};
    }

    et::Unary convert(const pt::Unary& src) {
      return et::Unary{.id = src.id, .location = src.location, .lhs = src.lhs, .op = src.op};
    }

    et::Call convert(const pt::Call& src) {
      return et::Call{.id = src.id,
                      .location = src.location,
                      .target = src.target,
                      ._return = src._return,
                      .arguments = src.arguments};
    }

    // Filled member by member: `indirect`'s default constructor is explicit, so the scopes cannot be brace-initialized.
    et::Conditional convert(const pt::Conditional& src) {
      et::Conditional out;
      out.id = src.id;
      out.location = src.location;
      out.condition = src.condition;
      out.inputs = src.inputs;
      out.outputs = src.outputs;
      *out.thenScope = convert(*src.thenScope);
      *out.elseScope = convert(*src.elseScope);
      return out;
    }

    et::Node convert(const pt::Node& src) {
      return std::visit([](const auto& node) { return et::Node{convert(node)}; }, src);
    }

    et::Block convert(const pt::Block& src) {
      et::Block out;
      for (const auto& [id, node] : src.nodes) {
        out.nodes.emplace(id, convert(node));
      }
      out.conduits = src.conduits;
      return out;
    }

    et::FunctionDecl convert(const pt::FunctionDecl& src) {
      et::FunctionDecl out;
      out.id = src.id;
      out.location = src.location;
      out.name = src.name;
      out.body = convert(src.body);
      out.input = src.input;
      out.output = src.output;
      return out;
    }

    et::Declaration convert(const pt::Declaration& src) {
      return std::visit([](const auto& decl) { return et::Declaration{convert(decl)}; }, src);
    }

  }  // namespace

  et::ParseTree annotate(const pt::ParseTree& tree) {
    et::ParseTree out;
    out.header = tree.header;
    for (const auto& [id, decl] : tree.declarations) {
      out.declarations.emplace(id, convert(decl));
    }
    return out;
  }

}  // namespace fluir::editor
