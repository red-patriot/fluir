#ifndef FLUIR_COMPILER_AFG_NODE_HPP
#define FLUIR_COMPILER_AFG_NODE_HPP

#include <concepts>
#include <memory>
#include <typeinfo>

#include "fluir/compiler/models/location.hpp"
#include "fluir/compiler/models/type.hpp"

namespace fluir::afg {
  class Node {
    friend bool operator==(const Node& lhs, const Node& rhs) {
      return lhs.where_ == rhs.where_ &&
             typeid(lhs) == typeid(rhs) &&
             lhs.equals(rhs);
    }

   public:
    Node() = default;
    explicit Node(LocationInfo location) :
        where_(location) { }

    virtual ~Node() = default;
    virtual Type type() const = 0;

    virtual bool equals(const Node& other) const = 0;

    template <typename NewSource>
      requires std::derived_from<NewSource, Node>
    NewSource* as() {
      auto concrete = dynamic_cast<NewSource*>(this);
      return concrete;
    }

    const LocationInfo& location() const { return where_; }

   private:
    LocationInfo where_{};
  };

  template <typename NewSource>
  concept ConcreteNode = std::derived_from<NewSource, fluir::afg::Node>;

  using UniqueNode = std::unique_ptr<Node>;
};  // namespace fluir::afg

#endif
