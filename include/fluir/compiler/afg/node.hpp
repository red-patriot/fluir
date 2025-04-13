#ifndef FLUIR_COMPILER_AFG_NODE_HPP
#define FLUIR_COMPILER_AFG_NODE_HPP

#include <concepts>
#include <memory>
#include <typeinfo>

#include "fluir/compiler/models/type.hpp"

namespace fluir::afg {
  class Node {
    friend bool operator==(const Node& lhs, const Node& rhs) {
      return typeid(lhs) == typeid(rhs) && lhs.equals(rhs);
    }

   public:
    virtual ~Node() = default;
    virtual Type type() const = 0;

    virtual bool equals(const Node& other) const = 0;

    template <typename NewSource>
      requires std::derived_from<NewSource, Node>
    NewSource* as() {
      auto concrete = dynamic_cast<NewSource*>(this);
      return concrete;
    }
  };

  template <typename NewSource>
  concept ConcreteNode = std::derived_from<NewSource, fluir::afg::Node>;

  using UniqueNode = std::unique_ptr<Node>;
};  // namespace fluir::afg

#endif
