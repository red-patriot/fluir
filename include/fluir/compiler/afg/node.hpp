#ifndef FLUIR_COMPILER_AFG_NODE_HPP
#define FLUIR_COMPILER_AFG_NODE_HPP

#include <concepts>
#include <memory>

#include "fluir/compiler/models/type.hpp"

namespace fluir::afg {
  class Node {
   public:
    virtual ~Node() = default;
    virtual Type type() const = 0;

    template <typename Concrete>
      requires std::derived_from<Concrete, Node>
    Concrete* as() {
      auto concrete = dynamic_cast<Concrete*>(this);
      return concrete;
    }
  };

  using SharedNode = std::shared_ptr<Node>;

  template <typename Concrete>
    requires std::derived_from<Concrete, Node>
  SharedNode shared(Concrete c) {
    return std::make_shared<Concrete>(std::move(c));
  }
};  // namespace fluir::afg

#endif
